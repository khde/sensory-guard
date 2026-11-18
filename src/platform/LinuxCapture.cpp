#include "LinuxCapture.h"

#ifdef SENSORYGUARD_HAS_X11

#include "platform/X11ErrorTrap.h"

#include <X11/Xutil.h>

#include <dlfcn.h>

// XComposite is loaded at runtime so its development headers are not a build dependency.
struct LinuxCompositeApi {
    using RedirectWindow = void (*)(Display *, Window, int);
    using UnredirectWindow = void (*)(Display *, Window, int);
    using NameWindowPixmap = Pixmap (*)(Display *, Window);

    LinuxCompositeApi() {
        library = dlopen("libXcomposite.so.1", RTLD_NOW | RTLD_LOCAL);
        if (library == nullptr)
            return;

        redirectWindow = reinterpret_cast<RedirectWindow>(dlsym(library, "XCompositeRedirectWindow"));
        unredirectWindow = reinterpret_cast<UnredirectWindow>(dlsym(library, "XCompositeUnredirectWindow"));
        nameWindowPixmap = reinterpret_cast<NameWindowPixmap>(dlsym(library, "XCompositeNameWindowPixmap"));
    }

    ~LinuxCompositeApi() {
        if (library != nullptr)
            dlclose(library);
    }

    LinuxCompositeApi(const LinuxCompositeApi &) = delete;
    LinuxCompositeApi &operator=(const LinuxCompositeApi &) = delete;

    bool isUsable() const {
        return redirectWindow != nullptr && unredirectWindow != nullptr && nameWindowPixmap != nullptr;
    }

    void *library = nullptr;
    RedirectWindow redirectWindow = nullptr;
    UnredirectWindow unredirectWindow = nullptr;
    NameWindowPixmap nameWindowPixmap = nullptr;
};

namespace {

constexpr int kCompositeRedirectAutomatic = 0;

unsigned char channelValue(unsigned long pixel, unsigned long mask) {
    if (mask == 0)
        return 0;

    unsigned int shift = 0;
    while (((mask >> shift) & 1u) == 0u)
        ++shift;

    const unsigned long value = (pixel & mask) >> shift;
    const unsigned long maximum = mask >> shift;
    return static_cast<unsigned char>((value * 255u) / maximum);
}

bool queryViewableWindow(Display *display, Window window, XWindowAttributes &attributes) {
    if (display == nullptr || window == 0)
        return false;

    X11ErrorTrap trap(display);
    return XGetWindowAttributes(display, window, &attributes) != 0 &&
        attributes.map_state == IsViewable && !trap.failed();
}

}

LinuxScreenCapture::LinuxScreenCapture(unsigned long targetWindow): m_targetWindow(targetWindow) {
    m_display = XOpenDisplay(nullptr);
    if (m_display == nullptr)
        return;

    auto composite = std::make_unique<LinuxCompositeApi>();
    if (!composite->isUsable())
        return;

    X11ErrorTrap trap(m_display);
    composite->redirectWindow(m_display, m_targetWindow, kCompositeRedirectAutomatic);
    if (trap.failed())
        return;

    m_composite = std::move(composite);
}

LinuxScreenCapture::~LinuxScreenCapture() {
    if (m_image != nullptr)
        XDestroyImage(m_image);

    if (m_display != nullptr) {
        releaseCapturePixmap();
        if (m_composite) {
            X11ErrorTrap trap(m_display);
            m_composite->unredirectWindow(m_display, m_targetWindow, kCompositeRedirectAutomatic);
        }
        XCloseDisplay(m_display);
    }
}

bool LinuxScreenCapture::isAvailable() const {
    XWindowAttributes attributes{};
    return queryViewableWindow(m_display, m_targetWindow, attributes);
}

bool LinuxScreenCapture::captureGeometry(int &x, int &y, int &width, int &height) const {
    XWindowAttributes attributes{};
    if (!queryViewableWindow(m_display, m_targetWindow, attributes))
        return false;

    X11ErrorTrap trap(m_display);
    Window child = 0;
    if (!XTranslateCoordinates(m_display, m_targetWindow, DefaultRootWindow(m_display), 0, 0, &x, &y, &child) ||
        trap.failed()) {
        return false;
    }

    width = attributes.width;
    height = attributes.height;
    return true;
}

void LinuxScreenCapture::releaseCapturePixmap() {
    if (m_capturePixmap != 0)
        XFreePixmap(m_display, m_capturePixmap);
    m_capturePixmap = 0;
}

void LinuxScreenCapture::refreshCapturePixmap(int width, int height) {
    if (!m_composite || (m_capturePixmap != 0 && m_pixmapWidth == width && m_pixmapHeight == height))
        return;

    releaseCapturePixmap();
    X11ErrorTrap trap(m_display);
    const Pixmap pixmap = m_composite->nameWindowPixmap(m_display, m_targetWindow);
    if (pixmap != 0 && !trap.failed()) {
        m_capturePixmap = pixmap;
        m_pixmapWidth = width;
        m_pixmapHeight = height;
    }
}

bool LinuxScreenCapture::captureFrame(cv::Mat &frame) {
    XWindowAttributes attributes{};
    if (!queryViewableWindow(m_display, m_targetWindow, attributes)) {
        m_lastCaptureError = "The selected X11 window is not viewable or no longer exists.";
        return false;
    }

    // Falls back to reading the window directly when XComposite is unavailable.
    refreshCapturePixmap(attributes.width, attributes.height);
    const Drawable source = m_capturePixmap != 0 ? m_capturePixmap : m_targetWindow;

    if (m_image != nullptr) {
        XDestroyImage(m_image);
        m_image = nullptr;
    }

    X11ErrorTrap trap(m_display);
    m_image = XGetImage(
        m_display,
        source,
        0,
        0,
        static_cast<unsigned int>(attributes.width),
        static_cast<unsigned int>(attributes.height),
        AllPlanes,
        ZPixmap);
    if (m_image == nullptr || trap.failed()) {
        m_lastCaptureError = "XGetImage failed (X11 error " + std::to_string(trap.errorCode()) + ").";
        if (m_image != nullptr) {
            XDestroyImage(m_image);
            m_image = nullptr;
        }
        return false;
    }
    m_lastCaptureError.clear();
    // XGetImage leaves the channel masks at 0 for pixmaps, so use the window's visual instead.
    unsigned long redMask = m_image->red_mask;
    unsigned long greenMask = m_image->green_mask;
    unsigned long blueMask = m_image->blue_mask;
    if (redMask == 0 && greenMask == 0 && blueMask == 0 && attributes.visual != nullptr) {
        redMask = attributes.visual->red_mask;
        greenMask = attributes.visual->green_mask;
        blueMask = attributes.visual->blue_mask;
    }

    frame.create(m_image->height, m_image->width, CV_8UC4);
    for (int y = 0; y < m_image->height; ++y) {
        for (int x = 0; x < m_image->width; ++x) {
            const unsigned long pixel = XGetPixel(m_image, x, y);
            cv::Vec4b &output = frame.at<cv::Vec4b>(y, x);
            output[0] = channelValue(pixel, blueMask);
            output[1] = channelValue(pixel, greenMask);
            output[2] = channelValue(pixel, redMask);
            output[3] = 255;
        }
    }

    return true;
}

std::string LinuxScreenCapture::lastCaptureError() const {
    return m_lastCaptureError;
}

#endif