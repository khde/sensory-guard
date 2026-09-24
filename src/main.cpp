#include <QApplication>
#include <QCoreApplication>
#include <QDir>

#include "engine/GuardEngine.h"
#include "gui/MainWindow.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    const QString modelPath = QDir(QCoreApplication::applicationDirPath()).filePath("data/models/640m.onnx");
    GuardEngine engine(modelPath.toStdString());
    MainWindow window(&engine);
    window.show();

    return app.exec();
}