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
    m_tabWidget->addTab(new SettingsTab(m_tabWidget), "Settings");
    m_tabWidget->setCurrentIndex(0);

    m_statusLabel->setText(m_controller->isActive() ? "Status: active" : "Status: disabled");
}
