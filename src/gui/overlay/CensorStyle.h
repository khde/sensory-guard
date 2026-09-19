#ifndef CENSORSTYLE_H
#define CENSORSTYLE_H

#include "gui/overlay/CensoringConfig.h"
#include "inference/Detection.h"

#include <opencv2/core/mat.hpp>

class CensorStyle {
public:
    CensorStyle();

    void setConfig(const CensoringConfig& config);
    CensoringStyle getStyle() const;
    void applyCensoring(cv::Mat& region);

private:
    void applyBlack(cv::Mat& region);
    void applyBlur(cv::Mat& region);
    void applyPixelate(cv::Mat& region);

    CensoringConfig m_config;
};

#endif
