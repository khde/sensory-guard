#include "StubShieldController.h"

#include <QTimer>
#include <cstdlib>

StubShieldController::StubShieldController(QObject *parent): IShieldController(parent), m_statsTimer(new QTimer(this)) {
    connect(m_statsTimer, &QTimer::timeout, this, &StubShieldController::tick);
    m_statsTimer->setInterval(1000);
}

void StubShieldController::start() {
    if (m_active)
        return;
    m_active = true;
    m_statsTimer->start();
    emit activeChanged(true);
}

void StubShieldController::stop() {
    if (!m_active)
        return;
    m_active = false;
    m_statsTimer->stop();
    emit activeChanged(false);
}

bool StubShieldController::isActive() const {
    return m_active;
}

void StubShieldController::tick() {
    m_censoredElements += std::rand() % 3;
    double fps = 55 + (std::rand() % 10);
    emit statsUpdated(m_censoredElements, fps);
}
