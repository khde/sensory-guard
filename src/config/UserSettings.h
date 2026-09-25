#ifndef USERSETTINGS_H
#define USERSETTINGS_H

#include <QStringList>

struct UserSettings {
    bool minimizeToTray = true;
    float sensitivity = 0.20f;
    QStringList enabledLabels;
    int maximumFps = 12;
    bool ignoreSmallScreenChanges = true;
    float frameChangeThreshold = 0.015f;
    int censorStyle = 0;
    int censorIntensity = 20;
    float censorScale = 1.0f;

    static UserSettings defaults();
    static UserSettings load();

    void save() const;
};

#endif