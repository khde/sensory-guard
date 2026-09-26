#include "ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QGuiApplication>
#include <QPalette>

namespace {
QString stylesheetPath(UserSettings::Theme theme) {
    if (theme == UserSettings::Theme::System) {
        const QPalette palette = QGuiApplication::palette();
        if (palette.color(QPalette::Window).lightness() < 128) {
            theme = UserSettings::Theme::Dark;
        } else {
            theme = UserSettings::Theme::Light;
        }
    }

    if (theme == UserSettings::Theme::Light) {
        return QStringLiteral(":/theme/light.qss");
    }

    return QStringLiteral(":/theme/dark.qss");
}
}

void ThemeManager::apply(UserSettings::Theme theme) {
    QFile stylesheet(stylesheetPath(theme));
    if (!stylesheet.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(QString());
        return;
    }

    qApp->setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
}
