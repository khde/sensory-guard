#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QIcon>

#include "engine/GuardEngine.h"
#include "gui/MainWindow.h"
#include "gui/theme/ThemeManager.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon(":/icons/logo.png"));
    QCoreApplication::setOrganizationName("SensorGuard");
    QCoreApplication::setApplicationName("SensorGuard");
    QCoreApplication::setApplicationVersion(SENSORGUARD_VERSION);

    const UserSettings userSettings = UserSettings::load();
    ThemeManager::apply(userSettings.theme);

    const QString modelPath = QDir(QCoreApplication::applicationDirPath()).filePath("data/models/640m.onnx");
    GuardEngine engine(modelPath.toStdString());
    MainWindow window(&engine);
    window.show();

    return app.exec();
}