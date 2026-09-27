#ifndef USERSETTINGS_H
#define USERSETTINGS_H

#include "engine/EngineConfig.h"

#include <QStringList>

struct UserSettings {
    enum class Theme {
        System = 0,
        Light = 1,
        Dark = 2
    };

    bool minimizeToTray = true;
    float sensitivity = 0.20f;
    QStringList enabledLabels;
    int maximumFps = 12;
    bool ignoreSmallScreenChanges = true;
    float frameChangeThreshold = 0.015f;
    int censorStyle = 0;
    int censorIntensity = 20;
    float censorScale = 1.0f;
    Theme theme = Theme::System;
    HardwareConfig hardware;
    DisplayConfig display;

    static UserSettings defaults();
    static UserSettings load();

    void save() const;
};

#endif