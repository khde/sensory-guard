#ifndef LINUX_CAPTURE_H
#define LINUX_CAPTURE_H

#ifdef SENSORYGUARD_HAS_X11

#include "platform/IScreenCapture.h"

#include <memory>
#include <string>

struct _XDisplay;
struct _XImage;
struct LinuxCompositeApi;

class LinuxScreenCapture final : public IScreenCapture {
public:
    explicit LinuxScreenCapture(unsigned long targetWindow);
    ~LinuxScreenCapture() override;

    bool isAvailable() const override;
    bool captureFrame(cv::Mat &frame) override;
    bool captureGeometry(int &x, int &y, int &width, int &height) const override;
    std::string lastCaptureError() const override;

private:
    void releaseCapturePixmap();
    void refreshCapturePixmap(int width, int height);

    _XDisplay *m_display = nullptr;
    unsigned long m_targetWindow = 0;
    _XImage *m_image = nullptr;
    std::unique_ptr<LinuxCompositeApi> m_composite;
    unsigned long m_capturePixmap = 0;
    int m_pixmapWidth = 0;
    int m_pixmapHeight = 0;
    std::string m_lastCaptureError;
};

#endif

#endif