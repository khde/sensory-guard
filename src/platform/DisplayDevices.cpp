#include "DisplayDevices.h"

#ifdef _WIN32

#include <windows.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include <cstdio>

using Microsoft::WRL::ComPtr;

namespace {

std::string narrow(const wchar_t *value) {
    if (value == nullptr || *value == L'\0') {
        return {};
    }

    const int size = WideCharToMultiByte(CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 1) {
        return {};
    }

    std::string result(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, value, -1, result.data(), size, nullptr, nullptr);
    result.resize(static_cast<size_t>(size - 1));
    return result;
}

std::string adapterId(const LUID &luid) {
    char value[32]{};
    std::snprintf(value, sizeof(value), "%08lx%08lx", static_cast<unsigned long>(luid.HighPart), static_cast<unsigned long>(luid.LowPart));
    return value;
}

std::string friendlyName(const wchar_t *deviceName, const std::string &fallback) {
    DISPLAY_DEVICEW monitor{};
    monitor.cb = sizeof(monitor);
    if (EnumDisplayDevicesW(deviceName, 0, &monitor, 0) && monitor.DeviceString[0] != L'\0') {
        return narrow(monitor.DeviceString);
    }
    return fallback;
}

}

std::vector<DisplayDescriptor> enumerateDisplays() {
    std::vector<DisplayDescriptor> displays;
    ComPtr<IDXGIFactory1> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
        return displays;
    }

    for (UINT adapterIndex = 0;; ++adapterIndex) {
        ComPtr<IDXGIAdapter1> adapter;
        if (factory->EnumAdapters1(adapterIndex, &adapter) == DXGI_ERROR_NOT_FOUND) {
            break;
        }

        DXGI_ADAPTER_DESC1 adapterDescription{};
        if (FAILED(adapter->GetDesc1(&adapterDescription))) {
            continue;
        }

        for (UINT outputIndex = 0;; ++outputIndex) {
            ComPtr<IDXGIOutput> output;
            if (adapter->EnumOutputs(outputIndex, &output) == DXGI_ERROR_NOT_FOUND) {
                break;
            }

            DXGI_OUTPUT_DESC outputDescription{};
            if (FAILED(output->GetDesc(&outputDescription))) {
                continue;
            }

            const std::string deviceName = narrow(outputDescription.DeviceName);
            DisplayDescriptor display;
            display.deviceName = deviceName;
            display.id = adapterId(adapterDescription.AdapterLuid) + "/" + deviceName;
            display.name = friendlyName(outputDescription.DeviceName, deviceName);
            display.desktopX = outputDescription.DesktopCoordinates.left;
            display.desktopY = outputDescription.DesktopCoordinates.top;
            display.desktopWidth = outputDescription.DesktopCoordinates.right - outputDescription.DesktopCoordinates.left;
            display.desktopHeight = outputDescription.DesktopCoordinates.bottom - outputDescription.DesktopCoordinates.top;
            display.adapterIndex = static_cast<int>(adapterIndex);
            display.outputIndex = static_cast<int>(outputIndex);
            display.captureWidth = display.desktopWidth;
            display.captureHeight = display.desktopHeight;
            displays.push_back(display);
        }
    }

    return displays;
}

bool resolveDisplay(const std::string &displayId, DisplayDescriptor &descriptor) {
    const std::vector<DisplayDescriptor> displays = enumerateDisplays();
    if (displayId.empty()) {
        for (const DisplayDescriptor &display : displays) {
            if (display.desktopX == 0 && display.desktopY == 0) {
                descriptor = display;
                return true;
            }
        }
        return false;
    }

    for (const DisplayDescriptor &display : displays) {
        if (display.id == displayId) {
            descriptor = display;
            return true;
        }
    }
    return false;
}

#elif defined(SENSORYGUARD_HAS_X11)

#include "platform/X11ErrorTrap.h"

#include <X11/Xatom.h>

#include <unistd.h>
#include <utility>

namespace {

std::string windowTitle(Display *display, Window window) {
    const Atom nameAtom = XInternAtom(display, "_NET_WM_NAME", True);
    const Atom utf8Atom = XInternAtom(display, "UTF8_STRING", True);
    if (nameAtom != None && utf8Atom != None) {
        Atom actualType = None;
        int actualFormat = 0;
        unsigned long itemCount = 0;
        unsigned long bytesAfter = 0;
        unsigned char *property = nullptr;
        if (XGetWindowProperty(
                display, window, nameAtom, 0, 1024, False, utf8Atom,
                &actualType, &actualFormat, &itemCount, &bytesAfter, &property) == Success &&
            property != nullptr && actualFormat == 8) {
            const std::string title(reinterpret_cast<char *>(property), itemCount);
            XFree(property);
            if (!title.empty())
                return title;
        } else if (property != nullptr) {
            XFree(property);
        }
    }

    char *legacyTitle = nullptr;
    if (XFetchName(display, window, &legacyTitle) == 0 || legacyTitle == nullptr)
        return {};
    const std::string title(legacyTitle);
    XFree(legacyTitle);
    return title;
}

bool belongsToCurrentProcess(Display *display, Window window) {
    const Atom pidAtom = XInternAtom(display, "_NET_WM_PID", True);
    if (pidAtom == None)
        return false;

    Atom actualType = None;
    int actualFormat = 0;
    unsigned long itemCount = 0;
    unsigned long bytesAfter = 0;
    unsigned char *property = nullptr;
    const int result = XGetWindowProperty(
        display, window, pidAtom, 0, 1, False, XA_CARDINAL,
        &actualType, &actualFormat, &itemCount, &bytesAfter, &property);
    if (result != Success || property == nullptr || actualFormat != 32 || itemCount == 0) {
        if (property != nullptr)
            XFree(property);
        return false;
    }

    const unsigned long processId = *reinterpret_cast<unsigned long *>(property);
    XFree(property);
    return processId == static_cast<unsigned long>(getpid());
}

void collectWindows(Display *display, Window parent, std::vector<DisplayDescriptor> &windows) {
    Window root = 0;
    Window parentReturn = 0;
    Window *children = nullptr;
    unsigned int childCount = 0;
    if (!XQueryTree(display, parent, &root, &parentReturn, &children, &childCount))
        return;

    for (unsigned int index = 0; index < childCount; ++index) {
        const Window window = children[index];
        XWindowAttributes attributes{};
        if (XGetWindowAttributes(display, window, &attributes) != 0 &&
            attributes.c_class == InputOutput &&
            attributes.map_state == IsViewable &&
            attributes.width >= 80 && attributes.height >= 50 &&
            !belongsToCurrentProcess(display, window)) {
            const std::string title = windowTitle(display, window);
            if (!title.empty()) {
                int rootX = 0;
                int rootY = 0;
                Window child = 0;
                if (XTranslateCoordinates(
                        display, window, DefaultRootWindow(display), 0, 0,
                        &rootX, &rootY, &child)) {
                    DisplayDescriptor descriptor;
                    descriptor.id = std::to_string(static_cast<unsigned long>(window));
                    descriptor.name = title;
                    descriptor.desktopX = rootX;
                    descriptor.desktopY = rootY;
                    descriptor.desktopWidth = attributes.width;
                    descriptor.desktopHeight = attributes.height;
                    descriptor.captureWidth = attributes.width;
                    descriptor.captureHeight = attributes.height;
                    descriptor.nativeWindowId = static_cast<unsigned long>(window);
                    windows.push_back(std::move(descriptor));
                }
            }
        }

        collectWindows(display, window, windows);
    }

    if (children != nullptr)
        XFree(children);
}

}

std::vector<DisplayDescriptor> enumerateDisplays() {
    std::vector<DisplayDescriptor> windows;
    Display *display = XOpenDisplay(nullptr);
    if (display == nullptr)
        return windows;

    {
        X11ErrorTrap trap(display);
        collectWindows(display, DefaultRootWindow(display), windows);
    }
    XCloseDisplay(display);
    return windows;
}

bool resolveDisplay(const std::string &displayId, DisplayDescriptor &descriptor) {
    const std::vector<DisplayDescriptor> windows = enumerateDisplays();
    if (displayId.empty()) {
        if (windows.empty())
            return false;
        descriptor = windows.front();
        return true;
    }

    for (const DisplayDescriptor &window : windows) {
        if (window.id == displayId) {
            descriptor = window;
            return true;
        }
    }
    return false;
}

#else

std::vector<DisplayDescriptor> enumerateDisplays() {
    return {};
}

bool resolveDisplay(const std::string &, DisplayDescriptor &) {
    return false;
}

#endif
