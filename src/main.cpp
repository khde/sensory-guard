#include <QApplication>

#include "engine/GuardEngine.h"
#include "gui/MainWindow.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    GuardEngine engine("data/models/640m.onnx");
    MainWindow window(&engine);
    window.show();

    return app.exec();
}