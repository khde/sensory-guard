#ifndef SETTINGSTAB_H
#define SETTINGSTAB_H

#include "config/UserSettings.h"

#include <QWidget>
#include <QCheckBox>
#include <QSlider>

class QComboBox;
class QLabel;
class QSpinBox;
class QPushButton;

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
    QComboBox *m_displayComboBox;
    QCheckBox *m_toggleHotkeyCheckBox;
    QPushButton *m_toggleHotkeyButton;
    QString m_toggleHotkey;

signals:
    void settingsChanged(bool minimizeToTray, bool ignoreSmallScreenChanges, float frameChangeThreshold, UserSettings::Theme theme, InferenceBackend backend, int deviceIndex, const QString &deviceId, int cpuThreadCount, const QString &displayId, bool toggleHotkeyEnabled, const QString &toggleHotkey);
    void resetRequested();
};

#endif