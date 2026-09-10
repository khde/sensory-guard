#pragma once

#include <QObject>

// Placeholder for real implementation
class IShieldController : public QObject {
    Q_OBJECT

public:
    using QObject::QObject;
    ~IShieldController() override = default;

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool isActive() const = 0;

signals:
    void activeChanged(bool active);
    void statsUpdated(int censoredElements, double fps);
    void failed(const QString &message);
};
