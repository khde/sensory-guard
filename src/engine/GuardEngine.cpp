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

	m_lastStatsTime = std::chrono::high_resolution_clock::now();
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
	
	std::vector<DetectionResult> emptyDetections;
	emit detectionsUpdated(emptyDetections);
	
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
	if (!m_capture->captureFrame(capturedFrame)) {
		// Clear stale detections when frame capture fails
		std::vector<DetectionResult> emptyDetections;
		emit detectionsUpdated(emptyDetections);
		return;
	}

	cv::Mat bgrFrame;
	if (capturedFrame.channels() == 4)
		cv::cvtColor(capturedFrame, bgrFrame, cv::COLOR_BGRA2BGR);
	else
		bgrFrame = capturedFrame;

	emit frameSizeChanged(bgrFrame.cols, bgrFrame.rows);
	emit frameCaptured(bgrFrame);
	std::vector<DetectionResult> detections = m_detector->detect(bgrFrame, m_config.confidenceThreshold);

	// Only keep user enabled classes
	detections.erase(
		std::remove_if(detections.begin(), detections.end(),
			[this](const DetectionResult &detection) {
				return m_config.enabledLabels.count(detection.entity) == 0;
			}),
		detections.end());

	emit detectionsUpdated(detections);
	m_censoredElements = static_cast<int>(detections.size());
	emit statsUpdated(m_censoredElements, m_config.maxFps);

	// Track FPS
	m_frameCount++;
	auto now = std::chrono::high_resolution_clock::now();
	auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastStatsTime).count();

	// Emit FPS around every 1000 ms
	if (elapsedMs >= 1000) {
		m_measuredFps = (m_frameCount * 1000.0) / elapsedMs;
		emit fpsUpdated(m_measuredFps, static_cast<double>(m_config.maxFps));

		// Reset counters
		m_frameCount = 0;
		m_lastStatsTime = now;
	}
}
