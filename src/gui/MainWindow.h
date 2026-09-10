#pragma once

#include <QMainWindow>
#include <QTabWidget>

class IShieldController;
class QLabel;
class QSystemTrayIcon;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(IShieldController *controller, QWidget *parent = nullptr);

private:
    void setupTrayIcon();

    IShieldController *m_controller;
    QTabWidget *m_tabWidget;
    QLabel *m_statusLabel;
    bool m_minimizeToTray = false;
    QSystemTrayIcon *m_trayIcon;

protected:
    void closeEvent(QCloseEvent *event) override;
};
