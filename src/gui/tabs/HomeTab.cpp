#include "HomeTab.h"

#include "engine/GuardEngine.h"

#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>
#include <QGroupBox>

HomeTab::HomeTab(GuardEngine *engine, QWidget *parent): QWidget(parent), m_engine(engine), m_activateButton(new QPushButton(this)), m_fpsLabel(new QLabel("Current: 0.00 FPS  |  Target: 0 FPS", this)) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *heading = new QLabel("GUARD OVERVIEW", this);
    heading->setProperty("role", "pageTitle");
    layout->addWidget(heading);

    m_activateButton->setMinimumHeight(54);
    layout->addWidget(m_activateButton);

    // System Stats
    QGroupBox *statsGroup = new QGroupBox("System Metrics", this);
    QVBoxLayout *statsLayout = new QVBoxLayout(statsGroup);
    statsLayout->addWidget(m_fpsLabel);
    layout->addWidget(statsGroup);

    layout->addStretch();

    connect(m_activateButton, &QPushButton::clicked, this, [this] {
        if (m_engine->isActive())
            m_engine->stop();
        else
            m_engine->start();
    });
    connect(m_engine, &GuardEngine::activeChanged, this, &HomeTab::updateActiveState);
    connect(m_engine, &GuardEngine::fpsUpdated, this, &HomeTab::updateFps);

    updateActiveState(m_engine->isActive());
}

void HomeTab::updateActiveState(bool active) {
    m_activateButton->setText(active ? "DEACTIVATE" : "ACTIVATE");
    m_activateButton->setProperty("active", active);
    m_activateButton->style()->unpolish(m_activateButton);
    m_activateButton->style()->polish(m_activateButton);
}

void HomeTab::updateFps(double currentFps, double targetFps) {
    m_fpsLabel->setText(QString("Current: %1 FPS  |  Target: %2 FPS").arg(currentFps, 0, 'f', 2).arg(static_cast<int>(targetFps)));
}
