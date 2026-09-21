#include "CensorStyle.h"

#include <opencv2/imgproc.hpp>
#include <algorithm>

#define BLUR_DOWNSAMPLE_FACTOR 0.05

CensorStyle::CensorStyle() = default;

void CensorStyle::setConfig(const CensoringConfig& config) {
	m_config = config;
}

CensoringStyle CensorStyle::getStyle() const {
	return m_config.style;
}

void CensorStyle::applyCensoring(cv::Mat& region) {
	if (region.empty())
		return;

	switch (m_config.style) {
		case CensoringStyle::Black:
			applyBlack(region);
			break;
		case CensoringStyle::GaussianBlur:
			applyGaussianBlur(region);
			break;
		case CensoringStyle::BoxBlur:
			applyBoxBlur(region);
			break;
		case CensoringStyle::Pixelation:
			applyPixelate(region);
			break;
	}
}

void CensorStyle::applyBlack(cv::Mat& region) {
	region = cv::Scalar(0, 0, 0);
}

void CensorStyle::applyGaussianBlur(cv::Mat& region) {
    if (region.empty())
        return;

    const int intensity = std::clamp(m_config.blurIntensity, 1, 100);
    const cv::Size originalSize = region.size();

    // Calculate the downscale factor
    double scale = 1.0;
    if (intensity > 20) {
        scale = 1.0 / (1.0 + (intensity - 20) * BLUR_DOWNSAMPLE_FACTOR);
    }

    cv::Mat smallRegion;
    cv::Size targetSize = originalSize;

    // Downscale the image with check against 0
    if (scale < 1.0) {
        int reducedWidth = std::max(1, cvRound(originalSize.width * scale));
        int reducedHeight = std::max(1, cvRound(originalSize.height * scale));
        targetSize = cv::Size(reducedWidth, reducedHeight);
        
        cv::resize(region, smallRegion, targetSize, 0, 0, cv::INTER_AREA);
    } else {
        smallRegion = region;
    }

    int kernelSize = 5 + (intensity / 4);

    kernelSize = std::min({kernelSize, targetSize.width, targetSize.height});
    
    // Kernel must be odd and at least 1
    if (kernelSize % 2 == 0) kernelSize--;
    if (kernelSize < 1) kernelSize = 1; 

    // Only blur if the kernel is larger than 1x1
    if (kernelSize > 1) {
        double sigma = kernelSize / 6.0;
        cv::GaussianBlur(smallRegion, smallRegion, cv::Size(kernelSize, kernelSize), sigma);
    }

    // Resize back to the original size
    if (scale < 1.0) {
        cv::resize(smallRegion, region, originalSize, 0, 0, cv::INTER_LINEAR);
    } else if (smallRegion.data != region.data) {
        smallRegion.copyTo(region);
    }
}


void CensorStyle::applyBoxBlur(cv::Mat& region) {
    if (region.empty())
        return;

    const int intensity = std::clamp(m_config.blurIntensity, 1, 100);
    
    int kernelSize = (intensity * 3 / 4) + 7;
    if (kernelSize % 2 == 0) kernelSize++;

    cv::blur(region, region, cv::Size(kernelSize, kernelSize));
}

void CensorStyle::applyPixelate(cv::Mat& region) {
	if (region.empty())
		return;

	int pixelSize = std::max(2, m_config.pixelSize);

	int smallWidth = std::max(1, region.cols / pixelSize);
	int smallHeight = std::max(1, region.rows / pixelSize);
	cv::Mat small;
	cv::resize(region, small, cv::Size(smallWidth, smallHeight), 0, 0, cv::INTER_LINEAR);

	cv::resize(small, region, cv::Size(region.cols, region.rows), 0, 0, cv::INTER_NEAREST);
}
