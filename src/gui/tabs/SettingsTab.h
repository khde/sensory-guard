#ifndef SETTINGSTAB_H
#define SETTINGSTAB_H

#include "config/UserSettings.h"

#include <QWidget>
#include <QCheckBox>
#include <QSlider>

class QComboBox;

class SettingsTab : public QWidget {
    Q_OBJECT

public:
    explicit SettingsTab(QWidget *parent = nullptr);
    void setSettings(const UserSettings &settings);

private:
	void updateFrameChangeLabel(int value);
	void updateFrameChangeControls();

    QCheckBox *m_minimizeToTrayCheckBox;
    QCheckBox *m_ignoreSmallScreenChangesCheckBox;
    QComboBox *m_themeComboBox;
    QLabel *m_frameChangeLabel;
    QSlider *m_frameChangeSlider;

signals:
    void settingsChanged(bool minimizeToTray, bool ignoreSmallScreenChanges, float frameChangeThreshold, UserSettings::Theme theme);
};

#endif