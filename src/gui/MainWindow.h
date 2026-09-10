#pragma once

#include <QMainWindow>
#include <QTabWidget>

class IShieldController;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(IShieldController *controller, QWidget *parent = nullptr);

private:
    void setupTabs();

    IShieldController *m_controller;
    QTabWidget *m_tabWidget;
};
