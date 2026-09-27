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
    settings.ignoreSmallScreenChanges = storage.value("ignoreSmallScreenChanges", fallback.ignoreSmallScreenChanges).toBool();
    settings.frameChangeThreshold = storage.value("frameChangeThreshold", fallback.frameChangeThreshold).toFloat();
    const int backend = storage.value("hardware/backend", static_cast<int>(fallback.hardware.backend)).toInt();
    if (backend >= static_cast<int>(InferenceBackend::Cpu) && backend <= static_cast<int>(InferenceBackend::DirectML)) {
        settings.hardware.backend = static_cast<InferenceBackend>(backend);
    }
#ifndef _WIN32
    settings.hardware.backend = InferenceBackend::Cpu;
#endif
    settings.hardware.deviceIndex = storage.value("hardware/deviceIndex", fallback.hardware.deviceIndex).toInt();
    settings.hardware.deviceId = storage.value("hardware/deviceId", QString::fromStdString(fallback.hardware.deviceId)).toString().toStdString();
    settings.hardware.cpuThreadCount = storage.value("hardware/cpuThreadCount", fallback.hardware.cpuThreadCount).toInt();
    if (settings.hardware.deviceIndex < 0) {
        settings.hardware.deviceIndex = fallback.hardware.deviceIndex;
    }
    if (settings.hardware.cpuThreadCount < 1) {
        settings.hardware.cpuThreadCount = fallback.hardware.cpuThreadCount;
    }
    settings.display.displayId = storage.value("display/displayId", QString::fromStdString(fallback.display.displayId)).toString().toStdString();
    settings.toggleHotkeyEnabled = storage.value("toggleHotkeyEnabled", storage.value("hotkeyEnabled", storage.value("globalHotkeyEnabled", fallback.toggleHotkeyEnabled))).toBool();
    settings.toggleHotkey = storage.value("toggleHotkey", storage.value("hotkey", storage.value("globalHotkey", fallback.toggleHotkey))).toString();
    const int theme = storage.value("theme", static_cast<int>(fallback.theme)).toInt();
    if (theme >= static_cast<int>(UserSettings::Theme::Dark) && theme <= static_cast<int>(UserSettings::Theme::System)) {
        settings.theme = static_cast<UserSettings::Theme>(theme);
    }

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
    storage.setValue("ignoreSmallScreenChanges", ignoreSmallScreenChanges);
    storage.setValue("frameChangeThreshold", frameChangeThreshold);
    storage.setValue("hardware/backend", static_cast<int>(hardware.backend));
    storage.setValue("hardware/deviceIndex", hardware.deviceIndex);
    storage.setValue("hardware/deviceId", QString::fromStdString(hardware.deviceId));
    storage.setValue("hardware/cpuThreadCount", hardware.cpuThreadCount);
    storage.setValue("display/displayId", QString::fromStdString(display.displayId));
    storage.setValue("toggleHotkeyEnabled", toggleHotkeyEnabled);
    storage.setValue("toggleHotkey", toggleHotkey);
    storage.setValue("theme", static_cast<int>(theme));
    storage.sync();
}