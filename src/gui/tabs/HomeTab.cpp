#include "HomeTab.h"

#include "backend/IShieldController.h"

#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

HomeTab::HomeTab(IShieldController *controller, QWidget *parent): QWidget(parent), m_controller(controller), m_activateButton(new QPushButton(this)) {
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
        if (m_controller->isActive())
            m_controller->stop();
        else
            m_controller->start();
    });
    connect(m_controller, &IShieldController::activeChanged, this, &HomeTab::updateActiveState);

    updateActiveState(m_controller->isActive());
}

void HomeTab::updateActiveState(bool active) {
    m_activateButton->setText(active ? "DEACTIVATE" : "ACTIVATE");
    m_activateButton->setProperty("active", active);
    m_activateButton->style()->unpolish(m_activateButton);
    m_activateButton->style()->polish(m_activateButton);
}
