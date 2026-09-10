#include "MainWindow.h"

#include <QWidget>

MainWindow::MainWindow(IShieldController *controller, QWidget *parent) : QMainWindow(parent), m_controller(controller), m_tabWidget(new QTabWidget(this)) {
    setWindowTitle("Sensory Guard");
    resize(390, 580);

    setCentralWidget(m_tabWidget);
    setupTabs();
}

void MainWindow::setupTabs() {
    m_tabWidget->addTab(new QWidget(m_tabWidget), "Home");
    m_tabWidget->addTab(new QWidget(m_tabWidget), "Detection");
    m_tabWidget->addTab(new QWidget(m_tabWidget), "Customize");
    m_tabWidget->addTab(new QWidget(m_tabWidget), "Settings");
}
