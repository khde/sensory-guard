#pragma once

#include "IShieldController.h"

class QTimer;

class StubShieldController : public IShieldController {
    Q_OBJECT

public:
    explicit StubShieldController(QObject *parent = nullptr);

    void start() override;
    void stop() override;
    bool isActive() const override;

private:
    void tick();

    bool m_active = false;
    int m_censoredElements = 0;
    QTimer *m_statsTimer;
};
