#include "LinuxCapture.h"

#ifdef SENSORGUARD_HAS_X11

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <algorithm>

namespace {

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

}

LinuxScreenCapture::LinuxScreenCapture() {
    m_display = XOpenDisplay(nullptr);
    if (m_display != nullptr)
        m_rootWindow = DefaultRootWindow(m_display);
}

LinuxScreenCapture::~LinuxScreenCapture() {
    if (m_image != nullptr)
        XDestroyImage(m_image);
    if (m_display != nullptr)
        XCloseDisplay(m_display);
}

bool LinuxScreenCapture::captureFrame(cv::Mat &frame) {
    if (m_display == nullptr || m_rootWindow == 0)
        return false;

    if (m_image != nullptr) {
        XDestroyImage(m_image);
        m_image = nullptr;
    }

    XWindowAttributes attributes{};
    if (XGetWindowAttributes(m_display, m_rootWindow, &attributes) == 0)
        return false;

    m_image = XGetImage(
        m_display,
        m_rootWindow,
        0,
        0,
        static_cast<unsigned int>(attributes.width),
        static_cast<unsigned int>(attributes.height),
        AllPlanes,
        ZPixmap);
    if (m_image == nullptr)
        return false;

    frame.create(m_image->height, m_image->width, CV_8UC4);
    for (int y = 0; y < m_image->height; ++y) {
        for (int x = 0; x < m_image->width; ++x) {
            const unsigned long pixel = XGetPixel(m_image, x, y);
            cv::Vec4b &output = frame.at<cv::Vec4b>(y, x);
            output[0] = channelValue(pixel, m_image->blue_mask);
            output[1] = channelValue(pixel, m_image->green_mask);
            output[2] = channelValue(pixel, m_image->red_mask);
            output[3] = 255;
        }
    }

    return true;
}

#endif