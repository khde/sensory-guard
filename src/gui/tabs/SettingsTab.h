#ifndef SETTINGSTAB_H
#define SETTINGSTAB_H

#include "config/UserSettings.h"

#include <QWidget>
#include <QCheckBox>
#include <QSlider>

class QComboBox;
class QSpinBox;

class SettingsTab : public QWidget {
    Q_OBJECT

public:
    explicit SettingsTab(QWidget *parent = nullptr);
    void setSettings(const UserSettings &settings);

private:
	void updateFrameChangeLabel(int value);
	void updateFrameChangeControls();
    void emitCurrentSettings();

    QCheckBox *m_minimizeToTrayCheckBox;
    QCheckBox *m_ignoreSmallScreenChangesCheckBox;
    QComboBox *m_themeComboBox;
    QLabel *m_frameChangeLabel;
    QSlider *m_frameChangeSlider;
    QComboBox *m_backendComboBox;
    QLabel *m_gpuLabel;
    QComboBox *m_gpuComboBox;
    QLabel *m_cpuThreadLabel;
    QSpinBox *m_cpuThreadSpinBox;

signals:
    void settingsChanged(bool minimizeToTray, bool ignoreSmallScreenChanges, float frameChangeThreshold, UserSettings::Theme theme, InferenceBackend backend, int deviceIndex, const QString &deviceId, int cpuThreadCount);
    void resetRequested();
};

#endif