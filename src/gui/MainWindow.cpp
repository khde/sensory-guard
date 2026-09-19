#include "MainWindow.h"

#include "tabs/HomeTab.h"
#include "tabs/DetectionTab.h"
#include "tabs/CustomizeTab.h"
#include "tabs/SettingsTab.h"
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

MainWindow::MainWindow(GuardEngine *engine, QWidget *parent): QMainWindow(parent), m_engine(engine), m_tabWidget(new QTabWidget(this)), m_statusLabel(new QLabel(this)), m_overlay(new CensorOverlay(nullptr)) {
    setWindowTitle("Sensory Guard");
    setFixedSize(390, 580);

    setCentralWidget(m_tabWidget);
    statusBar()->addWidget(m_statusLabel);
    statusBar()->addPermanentWidget(new QLabel("Sensory Guard v0.1.0", this));
    connect(m_engine, &GuardEngine::activeChanged, this, [this](bool active) {
        m_statusLabel->setText(active ? "Status: active" : "Status: disabled");
        m_statusLabel->setProperty("active", active);
        m_statusLabel->style()->unpolish(m_statusLabel);
        m_statusLabel->style()->polish(m_statusLabel);
    });
    connect(m_engine, &GuardEngine::frameSizeChanged, this, [this](int width, int height) {
        m_overlay->setSourceSize(QSize(width, height));
    });
    connect(m_engine, &GuardEngine::frameCaptured, m_overlay, &CensorOverlay::setSourceFrame);
    connect(m_engine, &GuardEngine::detectionsUpdated, m_overlay, &CensorOverlay::setDetections);
    connect(m_engine, &GuardEngine::statsUpdated, this, [this](int censoredElements, double) {
        m_statusLabel->setText(QString("Status: active (%1 regions)").arg(censoredElements));
    });
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
    DetectionTab *detectionTab = new DetectionTab(m_tabWidget);
    m_tabWidget->addTab(detectionTab, "Detection");
    connect(detectionTab, &DetectionTab::settingsChanged, this,
        [this](float confidenceThreshold, const QStringList &enabledLabels, int maxFps) {
            EngineConfig config = m_engine->config();
            config.confidenceThreshold = confidenceThreshold;
            config.maxFps = maxFps;
            config.enabledLabels.clear();
            for (const QString &label : enabledLabels)
                config.enabledLabels.insert(label.toStdString());
            m_engine->setConfig(config);
        });
    detectionTab->broadcastCurrentSettings();
    CustomizeTab *customizeTab = new CustomizeTab(m_tabWidget);
    m_tabWidget->addTab(customizeTab, "Customize");
    connect(customizeTab, &CustomizeTab::censoringConfigChanged, this,
        [this](const CensoringConfig& config) {
            m_overlay->setCensoringConfig(config);
        });
    customizeTab->broadcastCurrentSettings();
    SettingsTab *settingsTab = new SettingsTab(m_tabWidget);
    m_tabWidget->addTab(settingsTab, "Settings");
    connect(settingsTab, &SettingsTab::minimizeToTrayChanged, this, [this](bool enabled) {
        m_minimizeToTray = enabled;
    });
    m_minimizeToTray = settingsTab->minimizeToTrayEnabled();

    m_tabWidget->setCurrentIndex(0);

    m_statusLabel->setText(m_engine->isActive() ? "Status: active" : "Status: disabled");

    setupTrayIcon();
}

MainWindow::~MainWindow() {
    delete m_overlay;
}

void MainWindow::setupTrayIcon() {
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    m_trayIcon->setToolTip("Sensory Guard");

    QMenu *trayMenu = new QMenu(this);
    QAction *showAction = trayMenu->addAction("Open");
    QAction *quitAction = trayMenu->addAction("Quit");
    m_trayIcon->setContextMenu(trayMenu);

    connect(showAction, &QAction::triggered, this, [this] {
        show();
        raise();
        activateWindow();
    });
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    // Show window on left click
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            show();
            raise();
            activateWindow();
        }
    });

    m_trayIcon->show();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_minimizeToTray) {
        hide();
        event->ignore();
    }
    else {
        event->accept();
    }
}