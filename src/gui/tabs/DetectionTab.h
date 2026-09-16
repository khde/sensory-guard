#ifndef DETECTIONTAB_H
#define DETECTIONTAB_H

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

    void broadcastCurrentSettings();

signals:
    void settingsChanged(float confidenceThreshold, const QStringList &enabledLabels);

private:
    void updateSensitivityLabel(int value);
    QCheckBox *addLabelCheckbox(const QString &label, bool checked, QVBoxLayout *layout, QWidget *parent);
    void emitSettingsChanged();

    QLabel *sensitivityLabel;
    QSlider *sensitivitySlider;
    QVector<QCheckBox *> m_labelCheckBoxes;
};

#endif