#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "config/UserSettings.h"

class ThemeManager {
public:
    static void apply(UserSettings::Theme theme);
};

#endif
