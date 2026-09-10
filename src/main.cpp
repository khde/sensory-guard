#include <QApplication>

#include "backend/StubShieldController.h"
#include "gui/MainWindow.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    StubShieldController controller;
    MainWindow window(&controller);
    window.show();

    return app.exec();
}