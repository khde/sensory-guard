#ifndef DETECTIONTAB_H
#define DETECTIONTAB_H

#include <QWidget>
#include <QSlider>

class QLabel;

class DetectionTab : public QWidget {
    Q_OBJECT

public:
    explicit DetectionTab(QWidget *parent = nullptr);

private:
    void updateSensitivityLabel(int value);

    QLabel *sensitivityLabel;
    QSlider *sensitivitySlider;
};

#endif