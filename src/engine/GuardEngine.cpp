#include "GuardEngine.h"

#include "inference/OnnxDetector.h"

#ifdef _WIN32
	#include "platform/WindowsCapture.h"
#elif defined(SENSORGUARD_HAS_X11)
	#include "platform/LinuxCapture.h"
#endif

#include <opencv2/imgproc.hpp>
#include <QTimer>
#include <QString>
#include <algorithm>
#include <iostream>

GuardEngine::GuardEngine(const std::string &modelPath, QObject *parent)
	: QObject(parent),
	  m_statsTimer(new QTimer(this)),
	  m_detector(std::make_unique<OnnxDetector>(modelPath)) {
	#ifdef _WIN32
	m_capture = std::make_unique<WindowsScreenCapture>();
	#elif defined(SENSORGUARD_HAS_X11)
	m_capture = std::make_unique<LinuxScreenCapture>();
	#endif

	connect(m_statsTimer, &QTimer::timeout, this, &GuardEngine::tick);
	m_statsTimer->setInterval(1000 / m_config.maxFps);
}

GuardEngine::~GuardEngine() = default;

void GuardEngine::start() {
	if (m_active)
		return;

	m_active = true;
	m_statsTimer->start();
	emit activeChanged(true);
}

void GuardEngine::stop() {
	if (!m_active)
		return;

	m_active = false;
	m_statsTimer->stop();
	emit activeChanged(false);
}

bool GuardEngine::isActive() const {
	return m_active;
}

const EngineConfig &GuardEngine::config() const {
	return m_config;
}

void GuardEngine::setConfig(const EngineConfig &config) {
	m_config = config;
	m_statsTimer->setInterval(1000 / std::max(1, m_config.maxFps));
}

void GuardEngine::tick() {
	cv::Mat capturedFrame;
	if (!m_capture->captureFrame(capturedFrame))
		return;

	cv::Mat bgrFrame;
	if (capturedFrame.channels() == 4)
		cv::cvtColor(capturedFrame, bgrFrame, cv::COLOR_BGRA2BGR);
	else
		bgrFrame = capturedFrame;

	emit frameSizeChanged(bgrFrame.cols, bgrFrame.rows);
	std::vector<DetectionResult> detections = m_detector->detect(bgrFrame, m_config.confidenceThreshold);

	// Only keep user enabled classes
	detections.erase(
		std::remove_if(detections.begin(), detections.end(),
			[this](const DetectionResult &detection) {
				return m_config.enabledLabels.count(detection.entity) == 0;
			}),
		detections.end());

	std::cout << "[GuardEngine] captured " << bgrFrame.cols << "x" << bgrFrame.rows
			  << ", threshold=" << m_config.confidenceThreshold
			  << ", detections=" << detections.size() << std::endl;
	emit detectionsUpdated(detections);
	m_censoredElements = static_cast<int>(detections.size());
	emit statsUpdated(m_censoredElements, m_config.maxFps);
}
