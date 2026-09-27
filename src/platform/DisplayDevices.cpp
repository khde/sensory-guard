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

#else

std::vector<DisplayDescriptor> enumerateDisplays() {
    return {};
}

bool resolveDisplay(const std::string &, DisplayDescriptor &) {
    return false;
}

#endif
