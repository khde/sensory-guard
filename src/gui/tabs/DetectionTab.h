#ifndef DETECTIONTAB_H
#define DETECTIONTAB_H

#include "config/UserSettings.h"

#include <QWidget>
#include <QSlider>
#include <QStringList>
#include <QVector>

class QLabel;
class QCheckBox;
class QVBoxLayout;

class DetectionTab : public QWidget {
    Q_OBJECT

public:
    explicit DetectionTab(QWidget *parent = nullptr);

    void setSettings(const UserSettings &settings);

signals:
    void settingsChanged(float sensitivity, const QStringList &enabledLabels, int maximumFps);

private:
    void updateSensitivityLabel(int value);
    void updateFpsLabel(int value);
    QCheckBox *addLabelCheckbox(const QString &label, bool checked, QVBoxLayout *layout, QWidget *parent);
    void emitSettingsChanged();

    QLabel *sensitivityLabel;
    QSlider *sensitivitySlider;
    QLabel *fpsLabel;
    QSlider *fpsSlider;
    QVector<QCheckBox *> m_labelCheckBoxes;
};

#endif