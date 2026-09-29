#ifndef TOGGLE_HOTKEY_H
#define TOGGLE_HOTKEY_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QString>

class QWidget;
class QShortcut;

class ToggleHotkey final : public QObject
#ifdef _WIN32
    , public QAbstractNativeEventFilter
#endif
{
    Q_OBJECT

public:
    explicit ToggleHotkey(QWidget *window, QObject *parent = nullptr);
    ~ToggleHotkey() override;

    void update(bool enabled, const QString &shortcut);

signals:
    void activated();
    void error(const QString &message);

private:
#ifdef _WIN32
    bool registerWindowsHotkey(const QString &shortcut);
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
#endif
#ifdef SENSORYGUARD_HAS_X11
    bool registerX11Hotkey(const QString &shortcut);
    void handleX11Events();
    void unregisterX11Hotkey();
    struct _XDisplay *m_x11Display = nullptr;
    class QSocketNotifier *m_x11Notifier = nullptr;
    unsigned int m_x11Keycode = 0;
    unsigned int m_x11Modifiers = 0;
#endif

    void updateLocalShortcut(bool enabled, const QString &shortcut);

    QWidget *m_window;
    QShortcut *m_localShortcut;
    bool m_enabled = false;

};

#endif
