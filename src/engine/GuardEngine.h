#ifndef GUARDENGINE_H
#define GUARDENGINE_H

#include "inference/Detection.h"
#include "engine/EngineConfig.h"

#include <QObject>
#include <memory>
#include <string>
#include <vector>
#include <opencv2/core/mat.hpp>

class QTimer;
class OnnxDetector;
class IScreenCapture;

class GuardEngine : public QObject {
	Q_OBJECT

public:
	explicit GuardEngine(const std::string &modelPath, QObject *parent = nullptr);
	~GuardEngine() override;

	void start();
	void stop();
	bool isActive() const;

	const EngineConfig &config() const;
	void setConfig(const EngineConfig &config);

private:
	bool m_active = false;
	EngineConfig m_config;
	int m_censoredElements = 0;
	QTimer *m_statsTimer;
	std::unique_ptr<IScreenCapture> m_capture;
	std::unique_ptr<OnnxDetector> m_detector;

signals:
	void activeChanged(bool active);
	void statsUpdated(int censoredElements, double fps);
	void frameSizeChanged(int width, int height);
	void detectionsUpdated(const std::vector<DetectionResult> &detections);
	void frameCaptured(const cv::Mat &frame);
	void failed(const QString &message);

private slots:
	void tick();
};

#endif