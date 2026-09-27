#ifndef LINUX_CAPTURE_H
#define LINUX_CAPTURE_H

#ifdef SENSORGUARD_HAS_X11

#include "platform/IScreenCapture.h"

struct _XDisplay;
struct _XImage;

class LinuxScreenCapture final : public IScreenCapture {
public:
    LinuxScreenCapture();
    ~LinuxScreenCapture() override;

    bool isAvailable() const override;
    bool captureFrame(cv::Mat &frame) override;

private:
    _XDisplay *m_display = nullptr;
    unsigned long m_rootWindow = 0;
    _XImage *m_image = nullptr;
};

#endif

#endif