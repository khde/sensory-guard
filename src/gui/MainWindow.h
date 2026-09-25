#pragma once

#include "config/UserSettings.h"

#include <QMainWindow>
#include <QTabWidget>

class GuardEngine;
class CensorOverlay;
class AboutWindow;
class DetectionTab;
class CensorTab;
class SettingsTab;
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
    void applyUserSettings();
    void updateDetectionSettings(float sensitivity, const QStringList &enabledLabels, int maximumFps);
    void updateCensorSettings(int style, int intensity, float scale);
    void updateGeneralSettings(bool minimizeToTray, bool ignoreSmallScreenChanges, float frameChangeThreshold);

    GuardEngine *m_engine;
    UserSettings m_userSettings;
    AboutWindow *m_aboutWindow = nullptr;
    QTabWidget *m_tabWidget;
    DetectionTab *m_detectionTab;
    CensorTab *m_censorTab;
    SettingsTab *m_settingsTab;
    QLabel *m_statusLabel;
    bool m_minimizeToTray = false;
    QSystemTrayIcon *m_trayIcon;
    QAction *m_toggleAction = nullptr;
    CensorOverlay *m_overlay;

protected:
    void closeEvent(QCloseEvent *event) override;
};
