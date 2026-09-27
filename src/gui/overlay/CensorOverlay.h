#ifndef CENSOROVERLAY_H
#define CENSOROVERLAY_H

#include "inference/Detection.h"
#include "gui/overlay/CensoringConfig.h"

#include <QSize>
#include <QWidget>
#include <opencv2/core/mat.hpp>
#include <memory>
#include <vector>

class CensorStyle;

class CensorOverlay : public QWidget {
    Q_OBJECT

public:
    explicit CensorOverlay(QWidget *parent = nullptr);
    ~CensorOverlay() override;

    void setDetections(const std::vector<DetectionResult> &detections);
    void setSourceSize(const QSize &size);
    void setSourceFrame(const cv::Mat &frame);
    void setCensoringConfig(const CensoringConfig& config);
    void clearDetections();

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void applyX11OverlayHints();
    void paintBlackCensoringWithQt(QPainter &painter);
    void paintFrameWithCensoring(QPainter &painter);
    BoundingBox scaleBoundingBox(const BoundingBox &box, float factor) const;

    std::vector<DetectionResult> m_detections;
    QSize m_sourceSize;
    cv::Mat m_sourceFrame;
    std::unique_ptr<CensorStyle> m_censorStyle;
    float m_scaleFactor = 1.0f;
};

#endif