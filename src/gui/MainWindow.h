#pragma once

#include <QMainWindow>
#include <QTabWidget>

class GuardEngine;
class CensorOverlay;
class AboutWindow;
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
    void showAboutWindow();

    GuardEngine *m_engine;
    AboutWindow *m_aboutWindow = nullptr;
    QTabWidget *m_tabWidget;
    QLabel *m_statusLabel;
    bool m_minimizeToTray = false;
    QSystemTrayIcon *m_trayIcon;
    QAction *m_toggleAction = nullptr;
    CensorOverlay *m_overlay;

protected:
    void closeEvent(QCloseEvent *event) override;
};
