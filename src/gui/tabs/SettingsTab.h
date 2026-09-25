#ifndef SETTINGSTAB_H
#define SETTINGSTAB_H

#include "config/UserSettings.h"

#include <QWidget>
#include <QCheckBox>

class SettingsTab : public QWidget {
    Q_OBJECT

public:
    explicit SettingsTab(QWidget *parent = nullptr);
    void setSettings(const UserSettings &settings);

private:
    QCheckBox *m_minimizeToTrayCheckBox;

signals:
    void settingsChanged(bool minimizeToTray);
};

#endif