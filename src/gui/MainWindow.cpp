#include "MainWindow.h"

#include "tabs/HomeTab.h"
#include "tabs/DetectionTab.h"
#include "tabs/CustomizeTab.h"
#include "tabs/SettingsTab.h"
#include "backend/IShieldController.h"

#include <QLabel>
#include <QStyle>
#include <QStatusBar>
#include <QWidget>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QApplication>

MainWindow::MainWindow(IShieldController *controller, QWidget *parent): QMainWindow(parent), m_controller(controller), m_tabWidget(new QTabWidget(this)), m_statusLabel(new QLabel(this)) {
    setWindowTitle("Sensory Guard");
    setFixedSize(390, 580);

    setCentralWidget(m_tabWidget);
    statusBar()->addWidget(m_statusLabel);
    statusBar()->addPermanentWidget(new QLabel("Sensory Guard v0.1.0", this));
    connect(m_controller, &IShieldController::activeChanged, this, [this](bool active) {
        m_statusLabel->setText(active ? "Status: active" : "Status: disabled");
        m_statusLabel->setProperty("active", active);
        m_statusLabel->style()->unpolish(m_statusLabel);
        m_statusLabel->style()->polish(m_statusLabel);
    });

    // Tabs
    m_tabWidget->addTab(new HomeTab(m_controller, m_tabWidget), "Home");
    m_tabWidget->addTab(new DetectionTab(m_tabWidget), "Detection");
    m_tabWidget->addTab(new CustomizeTab(m_tabWidget), "Customize");
    SettingsTab *settingsTab = new SettingsTab(m_tabWidget);
    m_tabWidget->addTab(settingsTab, "Settings");
    connect(settingsTab, &SettingsTab::minimizeToTrayChanged, this, [this](bool enabled) {
        m_minimizeToTray = enabled;
    });
    m_minimizeToTray = settingsTab->minimizeToTrayEnabled();

    m_tabWidget->setCurrentIndex(0);

    m_statusLabel->setText(m_controller->isActive() ? "Status: active" : "Status: disabled");

    setupTrayIcon();
}

void MainWindow::setupTrayIcon()
{
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_minimizeToTray) {
        hide();
        event->ignore();
    }
    else {
        event->accept();
    }
}