#ifndef ISCREEN_CAPTURE_H
#define ISCREEN_CAPTURE_H

#include <opencv2/core.hpp>

class IScreenCapture {
public:
    virtual ~IScreenCapture() = default;

    virtual bool captureFrame(cv::Mat &frame) = 0;
};

#endif