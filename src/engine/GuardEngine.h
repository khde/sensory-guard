#ifndef GUARDENGINE_H
#define GUARDENGINE_H

#include "inference/Detection.h"
#include "engine/EngineConfig.h"
#include "platform/DisplayDevices.h"

#include <QObject>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <opencv2/core/mat.hpp>

class QTimer;
class OnnxDetector;
class IScreenCapture;

enum class EngineErrorCode {
	None,
	InvalidConfiguration,
	BackendUnavailable,
	DeviceUnavailable,
	ModelLoadFailed,
	CaptureUnavailable,
	DisplayUnavailable
};

struct EngineStartResult {
	bool success = false;
	EngineErrorCode errorCode = EngineErrorCode::None;
	std::string message;
};

class GuardEngine : public QObject {
	Q_OBJECT

public:
	explicit GuardEngine(const std::string &modelPath, QObject *parent = nullptr);
	~GuardEngine() override;

	EngineStartResult start();
	void stop();
	bool isActive() const;

	const EngineConfig &config() const;
	const DisplayDescriptor &display() const;
	void setConfig(const EngineConfig &config);

private:
	bool shouldRunInference(const cv::Mat &bgrFrame);

	bool m_active = false;
	std::string m_modelPath;
	EngineConfig m_config;
	DisplayDescriptor m_display;
	QTimer *m_statsTimer;
	std::unique_ptr<IScreenCapture> m_capture;
	std::unique_ptr<OnnxDetector> m_detector;
	cv::Mat m_previousComparisonFrame;
	cv::Mat m_currentGrayFrame;
	cv::Mat m_currentComparisonFrame;
	cv::Mat m_differenceFrame;
	std::vector<DetectionResult> m_lastDetections;
	bool m_frameProcessingIdle = false;

	// FPS tracking
	int m_frameCount = 0;
	std::chrono::high_resolution_clock::time_point m_lastStatsTime;
	double m_measuredFps = 0.0;

signals:
	void activeChanged(bool active);
	void fpsUpdated(double currentFps, double targetFps);
	void frameSizeChanged(int width, int height);
	void detectionsUpdated(const std::vector<DetectionResult> &detections);
	void frameCaptured(const cv::Mat &frame);
	void failed(const QString &message);

private slots:
	void tick();
};

#endif