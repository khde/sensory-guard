#include "MainWindow.h"

#include "tabs/HomeTab.h"
#include "tabs/DetectionTab.h"
#include "tabs/CensorTab.h"
#include "tabs/SettingsTab.h"
#include "AboutWindow.h"
#include "engine/GuardEngine.h"
#include "engine/EngineConfig.h"
#include "overlay/CensorOverlay.h"
#include "overlay/CensoringConfig.h"

#include <QLabel>
#include <QStyle>
#include <QStatusBar>
#include <QWidget>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QPushButton>
#include <QStringList>

#ifdef _WIN32
#include <windows.h>
#endif

MainWindow::MainWindow(GuardEngine *engine, QWidget *parent): QMainWindow(parent), m_engine(engine), m_tabWidget(new QTabWidget(this)), m_statusLabel(new QLabel(this)), m_overlay(new CensorOverlay(nullptr)) {
    setWindowTitle("Sensor Guard");
    setFixedSize(390, 580);

    m_userSettings = UserSettings::load();

    setCentralWidget(m_tabWidget);
    statusBar()->addWidget(m_statusLabel);
    QPushButton *aboutButton = new QPushButton("Sensor Guard v0.1.0", this);
    aboutButton->setFlat(true);
    aboutButton->setCursor(Qt::PointingHandCursor);
    statusBar()->addPermanentWidget(aboutButton);
    connect(aboutButton, &QPushButton::clicked, this, &MainWindow::showAboutWindow);
    connect(m_engine, &GuardEngine::activeChanged, this, [this](bool active) {
        m_statusLabel->setText(active ? "Status: active" : "Status: disabled");
        m_statusLabel->setProperty("active", active);
        m_statusLabel->style()->unpolish(m_statusLabel);
        m_statusLabel->style()->polish(m_statusLabel);
        if (m_toggleAction) {
            m_toggleAction->setText(active ? "Deactivate" : "Activate");
        }
    });
    connect(m_engine, &GuardEngine::frameSizeChanged, this, [this](int width, int height) {
        m_overlay->setSourceSize(QSize(width, height));
    });
    connect(m_engine, &GuardEngine::frameCaptured, m_overlay, &CensorOverlay::setSourceFrame);
    connect(m_engine, &GuardEngine::detectionsUpdated, m_overlay, &CensorOverlay::setDetections);
    connect(m_engine, &GuardEngine::activeChanged, m_overlay, [this](bool active) {
        if (active) {
            if (QScreen *screen = QGuiApplication::primaryScreen())
                m_overlay->setGeometry(screen->geometry());
            m_overlay->show();
            m_overlay->raise();
        } else {
            m_overlay->hide();
        }
    });

    if (QScreen *screen = QGuiApplication::primaryScreen())
        m_overlay->setGeometry(screen->geometry());

    // Tabs
    m_tabWidget->addTab(new HomeTab(m_engine, m_tabWidget), "Home");
    m_detectionTab = new DetectionTab(m_tabWidget);
    m_tabWidget->addTab(m_detectionTab, "Detection");
    m_detectionTab->setSettings(m_userSettings);
    connect(m_detectionTab, &DetectionTab::settingsChanged, this, &MainWindow::updateDetectionSettings);
    m_censorTab = new CensorTab(m_tabWidget);
    m_tabWidget->addTab(m_censorTab, "Censor");
    m_censorTab->setSettings(m_userSettings);
    connect(m_censorTab, &CensorTab::settingsChanged, this, &MainWindow::updateCensorSettings);
    m_settingsTab = new SettingsTab(m_tabWidget);
    m_tabWidget->addTab(m_settingsTab, "Settings");
    m_settingsTab->setSettings(m_userSettings);
    connect(m_settingsTab, &SettingsTab::settingsChanged, this, &MainWindow::updateGeneralSettings);

    applyUserSettings();

    m_tabWidget->setCurrentIndex(0);

    m_statusLabel->setText(m_engine->isActive() ? "Status: active" : "Status: disabled");

    setupTrayIcon();
}

MainWindow::~MainWindow() {
    delete m_overlay;
}

void MainWindow::applyUserSettings() {
    EngineConfig engineConfig = m_engine->config();
    engineConfig.confidenceThreshold = m_userSettings.sensitivity;
    engineConfig.maxFps = m_userSettings.maximumFps;
    engineConfig.enabledLabels.clear();
    for (const QString &label : m_userSettings.enabledLabels) {
        engineConfig.enabledLabels.insert(label.toStdString());
    }
    m_engine->setConfig(engineConfig);

    CensoringConfig censoringConfig;
    censoringConfig.style = static_cast<CensoringStyle>(m_userSettings.censorStyle);
    censoringConfig.blurIntensity = m_userSettings.censorIntensity;
    censoringConfig.pixelSize = m_userSettings.censorIntensity;
    censoringConfig.scaleFactor = m_userSettings.censorScale;
    m_overlay->setCensoringConfig(censoringConfig);

    m_minimizeToTray = m_userSettings.minimizeToTray;
}

void MainWindow::updateDetectionSettings(float sensitivity, const QStringList &enabledLabels, int maximumFps) {
    m_userSettings.sensitivity = sensitivity;
    m_userSettings.enabledLabels = enabledLabels;
    m_userSettings.maximumFps = maximumFps;
    applyUserSettings();
    m_userSettings.save();
}

void MainWindow::updateCensorSettings(int style, int intensity, float scale) {
    m_userSettings.censorStyle = style;
    m_userSettings.censorIntensity = intensity;
    m_userSettings.censorScale = scale;
    applyUserSettings();
    m_userSettings.save();
}

void MainWindow::updateGeneralSettings(bool minimizeToTray) {
    m_userSettings.minimizeToTray = minimizeToTray;
    applyUserSettings();
    m_userSettings.save();
}

void MainWindow::setupTrayIcon() {
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    m_trayIcon->setToolTip("Sensor Guard");

    QMenu *trayMenu = new QMenu(this);
    QAction *showAction = trayMenu->addAction("Sensor Guard");
    trayMenu->addSeparator();
    m_toggleAction = trayMenu->addAction(m_engine->isActive() ? "Deactivate" : "Activate");
    trayMenu->addSeparator();
    QAction *exitAction = trayMenu->addAction("Exit");
    m_trayIcon->setContextMenu(trayMenu);

    connect(showAction, &QAction::triggered, this, &MainWindow::bringToForeground);
    connect(m_toggleAction, &QAction::triggered, this, [this] {
        if (m_engine->isActive()) {
            m_engine->stop();
        } else {
            m_engine->start();
        }
    });
    connect(exitAction, &QAction::triggered, qApp, &QApplication::exit);

    // Bring the window to front on left click
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            bringToForeground();
        }
    });

    m_trayIcon->show();
}

void MainWindow::bringToForeground() {
    if (isMinimized()) {
        showNormal();
    }
    else {
        show();
    }
    raise();
    activateWindow();

#ifdef _WIN32
    // Windows might not put it in the foreground
    HWND targetWindow = reinterpret_cast<HWND>(winId());
    HWND foregroundWindow = GetForegroundWindow();
    DWORD targetThreadId = GetCurrentThreadId();
    DWORD foregroundThreadId = foregroundWindow ? GetWindowThreadProcessId(foregroundWindow, nullptr) : 0;

    bool attached = foregroundThreadId && foregroundThreadId != targetThreadId && AttachThreadInput(foregroundThreadId, targetThreadId, TRUE);
    SetForegroundWindow(targetWindow);
    if (attached) {
        AttachThreadInput(foregroundThreadId, targetThreadId, FALSE);
    }
#endif
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_minimizeToTray) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::showAboutWindow() {
    if (!m_aboutWindow) {
        m_aboutWindow = new AboutWindow("0.1.0", this);
    }
    m_aboutWindow->show();
    m_aboutWindow->raise();
    m_aboutWindow->activateWindow();
}