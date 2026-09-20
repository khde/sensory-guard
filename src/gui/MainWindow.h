#pragma once

#include <QMainWindow>
#include <QTabWidget>

class GuardEngine;
class CensorOverlay;
class QLabel;
class QSystemTrayIcon;
class QAction;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(GuardEngine *engine, QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void setupTrayIcon();
    void bringToForeground();

    GuardEngine *m_engine;
    QTabWidget *m_tabWidget;
    QLabel *m_statusLabel;
    bool m_minimizeToTray = false;
    QSystemTrayIcon *m_trayIcon;
    QAction *m_toggleAction = nullptr;
    CensorOverlay *m_overlay;

protected:
    void closeEvent(QCloseEvent *event) override;
};
