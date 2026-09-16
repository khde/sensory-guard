#ifndef CENSOROVERLAY_H
#define CENSOROVERLAY_H

#include "inference/Detection.h"

#include <QSize>
#include <QWidget>
#include <vector>

class CensorOverlay : public QWidget {
    Q_OBJECT

public:
    explicit CensorOverlay(QWidget *parent = nullptr);

    void setDetections(const std::vector<DetectionResult> &detections);
    void setSourceSize(const QSize &size);
    void clearDetections();

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void applyX11OverlayHints();

    std::vector<DetectionResult> m_detections;
    QSize m_sourceSize;
};

#endif