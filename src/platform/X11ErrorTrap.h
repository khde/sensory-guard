#ifndef X11_ERROR_TRAP_H
#define X11_ERROR_TRAP_H

#ifdef SENSORYGUARD_HAS_X11

// Include after any Qt headers: Xlib defines macros such as None and Bool that break them.
#include <X11/Xlib.h>

// Swallows X protocol errors while alive, so requests against windows that vanish mid-call don't abort the process.
// Not nestable: errors are tracked in one process-wide slot.
class X11ErrorTrap {
public:
    explicit X11ErrorTrap(Display *display): m_display(display) {
        XSync(m_display, False);
        errorSlot() = 0;
        m_previousHandler = XSetErrorHandler(recordError);
    }

    ~X11ErrorTrap() {
        XSync(m_display, False);
        XSetErrorHandler(m_previousHandler);
    }

    X11ErrorTrap(const X11ErrorTrap &) = delete;
    X11ErrorTrap &operator=(const X11ErrorTrap &) = delete;

    int errorCode() const {
        XSync(m_display, False);
        return errorSlot();
    }

    bool failed() const {
        return errorCode() != 0;
    }

private:
    static int &errorSlot() {
        static int code = 0;
        return code;
    }

    static int recordError(Display *, XErrorEvent *event) {
        errorSlot() = event->error_code;
        return 0;
    }

    Display *m_display;
    XErrorHandler m_previousHandler = nullptr;
};

#endif

#endif
