#include "ToggleHotkey.h"

#include <QApplication>
#include <QKeySequence>
#include <QShortcut>
#include <QWidget>

#ifdef _WIN32
#include <windows.h>

namespace {
constexpr int kHotkeyId = 1;

bool convertWindowsHotkey(const QString &shortcut, UINT &modifiers, UINT &virtualKey) {
    const QKeySequence sequence = QKeySequence::fromString(shortcut, QKeySequence::PortableText);
    if (sequence.count() != 1) {
        return false;
    }

    const QKeyCombination combination = sequence[0];
    const Qt::KeyboardModifiers qtModifiers = combination.keyboardModifiers();
    if (qtModifiers == Qt::NoModifier) {
        return false;
    }

    modifiers = MOD_NOREPEAT;
    if (qtModifiers & Qt::ControlModifier) {
        modifiers |= MOD_CONTROL;
    }
    if (qtModifiers & Qt::AltModifier) {
        modifiers |= MOD_ALT;
    }
    if (qtModifiers & Qt::ShiftModifier) {
        modifiers |= MOD_SHIFT;
    }
    if (qtModifiers & Qt::MetaModifier) {
        modifiers |= MOD_WIN;
    }

    const int key = combination.key();
    if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        virtualKey = static_cast<UINT>(key);
        return true;
    }
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        virtualKey = static_cast<UINT>(key);
        return true;
    }
    if (key >= Qt::Key_F1 && key <= Qt::Key_F24) {
        virtualKey = static_cast<UINT>(VK_F1 + (key - Qt::Key_F1));
        return true;
    }

    switch (key) {
        case Qt::Key_Space:
            virtualKey = VK_SPACE;
            return true;
        case Qt::Key_Tab:
            virtualKey = VK_TAB;
            return true;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            virtualKey = VK_RETURN;
            return true;
        case Qt::Key_Escape:
            virtualKey = VK_ESCAPE;
            return true;
        default:
            return false;
    }
}
}
#endif

#ifdef SENSORGUARD_HAS_X11
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <QSocketNotifier>

namespace {
bool convertX11Hotkey(const QString &shortcut, Display *display, unsigned int &modifiers, unsigned int &keycode) {
    const QKeySequence sequence = QKeySequence::fromString(shortcut, QKeySequence::PortableText);
    if (sequence.count() != 1) {
        return false;
    }

    const QKeyCombination combination = sequence[0];
    const Qt::KeyboardModifiers qtModifiers = combination.keyboardModifiers();
    if (qtModifiers == Qt::NoModifier) {
        return false;
    }

    modifiers = 0;
    if (qtModifiers & Qt::ControlModifier) {
        modifiers |= ControlMask;
    }
    if (qtModifiers & Qt::AltModifier) {
        modifiers |= Mod1Mask;
    }
    if (qtModifiers & Qt::ShiftModifier) {
        modifiers |= ShiftMask;
    }
    if (qtModifiers & Qt::MetaModifier) {
        modifiers |= Mod4Mask;
    }

    const int key = combination.key();
    QString keyName;
    if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        keyName = QChar(static_cast<ushort>(key)).toUpper();
    } else if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        keyName = QChar(static_cast<ushort>(key));
    } else if (key >= Qt::Key_F1 && key <= Qt::Key_F24) {
        keyName = QString("F%1").arg(key - Qt::Key_F1 + 1);
    } else {
        switch (key) {
            case Qt::Key_Space:
                keyName = "space";
                break;
            case Qt::Key_Tab:
                keyName = "Tab";
                break;
            case Qt::Key_Return:
            case Qt::Key_Enter:
                keyName = "Return";
                break;
            case Qt::Key_Escape:
                keyName = "Escape";
                break;
            default:
                return false;
        }
    }

    const QByteArray keyNameBytes = keyName.toLatin1();
    const KeySym keySym = XStringToKeysym(keyNameBytes.constData());
    if (keySym == NoSymbol) {
        return false;
    }

    keycode = XKeysymToKeycode(display, keySym);
    return keycode != 0;
}
}
#endif

ToggleHotkey::ToggleHotkey(QWidget *window, QObject *parent)
    : QObject(parent),
      m_window(window),
      m_localShortcut(new QShortcut(window)) {
    m_localShortcut->setContext(Qt::ApplicationShortcut);
        m_localShortcut->setEnabled(false);
    connect(m_localShortcut, &QShortcut::activated, this, &ToggleHotkey::activated);
#ifdef _WIN32
    qApp->installNativeEventFilter(this);
#endif
}

ToggleHotkey::~ToggleHotkey() {
    update(false, {});
#ifdef _WIN32
    qApp->removeNativeEventFilter(this);
#endif
}

void ToggleHotkey::update(bool enabled, const QString &shortcut) {
    m_localShortcut->setEnabled(false);
#ifdef _WIN32
    HWND window = reinterpret_cast<HWND>(m_window->winId());
    UnregisterHotKey(window, kHotkeyId);
    m_enabled = false;

    if (enabled && !registerWindowsHotkey(shortcut)) {
        return;
    }
#elif defined(SENSORGUARD_HAS_X11)
    unregisterX11Hotkey();
    m_enabled = false;

    if (enabled && !registerX11Hotkey(shortcut)) {
        return;
    }
#else
    updateLocalShortcut(enabled, shortcut);
#endif

    m_enabled = enabled;
}

void ToggleHotkey::updateLocalShortcut(bool enabled, const QString &shortcut) {
    m_localShortcut->setKey(QKeySequence::fromString(shortcut, QKeySequence::PortableText));
    m_localShortcut->setEnabled(enabled && !m_localShortcut->key().isEmpty());
}

#ifdef _WIN32
bool ToggleHotkey::registerWindowsHotkey(const QString &shortcut) {
    UINT modifiers = 0;
    UINT virtualKey = 0;
    if (!convertWindowsHotkey(shortcut, modifiers, virtualKey)) {
        emit error("The configured shortcut is not supported.");
        return false;
    }

    HWND window = reinterpret_cast<HWND>(m_window->winId());
    if (!RegisterHotKey(window, kHotkeyId, modifiers, virtualKey)) {
        emit error("The configured shortcut is already in use by another application.");
        return false;
    }
    return true;
}

bool ToggleHotkey::nativeEventFilter(const QByteArray &, void *message, qintptr *result) {
    MSG *nativeMessage = static_cast<MSG *>(message);
    if (nativeMessage->message == WM_HOTKEY && nativeMessage->wParam == kHotkeyId) {
        emit activated();
        if (result != nullptr) {
            *result = 0;
        }
        return true;
    }
    return false;
}
#endif

#ifdef SENSORGUARD_HAS_X11
bool ToggleHotkey::registerX11Hotkey(const QString &shortcut) {
    Display *display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        emit error("The X11 display could not be opened.");
        return false;
    }

    unsigned int modifiers = 0;
    unsigned int keycode = 0;
    if (!convertX11Hotkey(shortcut, display, modifiers, keycode)) {
        XCloseDisplay(display);
        emit error("The configured shortcut is not supported on X11.");
        return false;
    }

    const Window root = DefaultRootWindow(display);
    const unsigned int lockMasks[] = {0, LockMask, Mod2Mask, LockMask | Mod2Mask};
    for (const unsigned int lockMask : lockMasks) {
        XGrabKey(display, static_cast<int>(keycode), modifiers | lockMask, root, False, GrabModeAsync, GrabModeAsync);
    }
    XSync(display, False);

    m_x11Display = display;
    m_x11Keycode = keycode;
    m_x11Modifiers = modifiers;
    m_x11Notifier = new QSocketNotifier(ConnectionNumber(display), QSocketNotifier::Read, this);
    connect(m_x11Notifier, &QSocketNotifier::activated, this, [this] {
        handleX11Events();
    });
    return true;
}

void ToggleHotkey::unregisterX11Hotkey() {
    if (m_x11Display == nullptr) {
        return;
    }

    const Window root = DefaultRootWindow(m_x11Display);
    const unsigned int lockMasks[] = {0, LockMask, Mod2Mask, LockMask | Mod2Mask};
    for (const unsigned int lockMask : lockMasks) {
        XUngrabKey(m_x11Display, static_cast<int>(m_x11Keycode), m_x11Modifiers | lockMask, root);
    }
    XCloseDisplay(m_x11Display);
    m_x11Display = nullptr;
    delete m_x11Notifier;
    m_x11Notifier = nullptr;
    m_x11Keycode = 0;
    m_x11Modifiers = 0;
}

void ToggleHotkey::handleX11Events() {
    if (m_x11Display == nullptr) {
        return;
    }

    while (XPending(m_x11Display) > 0) {
        XEvent event{};
        XNextEvent(m_x11Display, &event);
        if (event.type != KeyPress) {
            continue;
        }

        const XKeyEvent &keyEvent = event.xkey;
        if (keyEvent.keycode == m_x11Keycode &&
            (keyEvent.state & m_x11Modifiers) == m_x11Modifiers) {
            emit activated();
        }
    }
}
#endif
