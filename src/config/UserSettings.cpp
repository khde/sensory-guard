#include "UserSettings.h"

#include <QSettings>

namespace {
const QStringList DEFAULT_ENABLED_LABELS = {
    "FEMALE_GENITALIA_EXPOSED",
    "FEMALE_BREAST_EXPOSED",
    "BUTTOCKS_EXPOSED",
    "MALE_GENITALIA_EXPOSED",
    "MALE_BREAST_EXPOSED",
    "ANUS_EXPOSED",
    "BELLY_EXPOSED",
    "ARMPITS_EXPOSED",
    "FEET_EXPOSED"
};
}

UserSettings UserSettings::defaults() {
    UserSettings settings;
    settings.enabledLabels = DEFAULT_ENABLED_LABELS;
    return settings;
}

UserSettings UserSettings::load() {
    const UserSettings fallback = defaults();
    UserSettings settings = fallback;
    QSettings storage;

    settings.sensitivity = storage.value("sensitivity", fallback.sensitivity).toFloat();
    settings.maximumFps = storage.value("maximumFps", fallback.maximumFps).toInt();
    settings.enabledLabels = storage.value("enabledLabels", fallback.enabledLabels).toStringList();
    settings.censorStyle = storage.value("censorStyle", fallback.censorStyle).toInt();
    settings.censorIntensity = storage.value("censorIntensity", fallback.censorIntensity).toInt();
    settings.censorScale = storage.value("censorScale", fallback.censorScale).toFloat();
    settings.minimizeToTray = storage.value("minimizeToTray", fallback.minimizeToTray).toBool();

    return settings;
}

void UserSettings::save() const {
    QSettings storage;
    storage.setValue("sensitivity", sensitivity);
    storage.setValue("maximumFps", maximumFps);
    storage.setValue("enabledLabels", enabledLabels);
    storage.setValue("censorStyle", censorStyle);
    storage.setValue("censorIntensity", censorIntensity);
    storage.setValue("censorScale", censorScale);
    storage.setValue("minimizeToTray", minimizeToTray);
    storage.sync();
}