#include "CensorStyle.h"

#include <opencv2/imgproc.hpp>
#include <algorithm>

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
			applyBlur(region);
			break;
		case CensoringStyle::Pixelation:
			applyPixelate(region);
			break;
	}
}

void CensorStyle::applyBlack(cv::Mat& region) {
	region = cv::Scalar(0, 0, 0);
}

void CensorStyle::applyBlur(cv::Mat& region) {
	if (region.empty())
		return;

	int kernelSize = (m_config.blurIntensity * 3 / 2) + 1;
	kernelSize = std::max(7, kernelSize);
	if (kernelSize % 2 == 0) kernelSize++;

	double sigma = kernelSize / 6.0;
	cv::GaussianBlur(region, region, cv::Size(kernelSize, kernelSize), sigma);
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
