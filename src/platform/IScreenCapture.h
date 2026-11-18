#ifndef ISCREEN_CAPTURE_H
#define ISCREEN_CAPTURE_H

#include <opencv2/core.hpp>
#include <string>

class IScreenCapture {
public:
    virtual ~IScreenCapture() = default;

    virtual bool isAvailable() const = 0;
    virtual bool captureFrame(cv::Mat &frame) = 0;
    virtual bool captureGeometry(int &, int &, int &, int &) const { return false; }
    virtual std::string lastCaptureError() const { return {}; }
};

#endif