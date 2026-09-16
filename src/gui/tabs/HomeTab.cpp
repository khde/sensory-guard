#include "HomeTab.h"

#include "engine/GuardEngine.h"

#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

HomeTab::HomeTab(GuardEngine *engine, QWidget *parent): QWidget(parent), m_engine(engine), m_activateButton(new QPushButton(this)) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *heading = new QLabel("GUARD OVERVIEW", this);
    layout->addWidget(heading);

    QWidget *shieldCard = new QWidget(this);
    QVBoxLayout *shieldLayout = new QVBoxLayout(shieldCard);
    shieldLayout->setContentsMargins(12, 12, 12, 12);
    shieldLayout->setSpacing(10);

    m_activateButton->setMinimumHeight(54);
    shieldLayout->addWidget(m_activateButton);
    layout->addWidget(shieldCard);
    layout->addStretch();

    connect(m_activateButton, &QPushButton::clicked, this, [this] {
        if (m_engine->isActive())
            m_engine->stop();
        else
            m_engine->start();
    });
    connect(m_engine, &GuardEngine::activeChanged, this, &HomeTab::updateActiveState);

    updateActiveState(m_engine->isActive());
}

void HomeTab::updateActiveState(bool active) {
    m_activateButton->setText(active ? "DEACTIVATE" : "ACTIVATE");
    m_activateButton->setProperty("active", active);
    m_activateButton->style()->unpolish(m_activateButton);
    m_activateButton->style()->polish(m_activateButton);
}
