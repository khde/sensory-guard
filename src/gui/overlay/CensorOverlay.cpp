#include "CensorOverlay.h"

#include <QPainter>
#include <QPaintEvent>
#include <QShowEvent>

#ifdef SENSORGUARD_HAS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#endif

CensorOverlay::CensorOverlay(QWidget *parent): QWidget(parent) {
	setWindowFlags(Qt::Tool |
				   Qt::FramelessWindowHint |
				   Qt::WindowStaysOnTopHint |
				   Qt::WindowDoesNotAcceptFocus |
				   Qt::X11BypassWindowManagerHint);
	setAttribute(Qt::WA_NativeWindow);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setAttribute(Qt::WA_ShowWithoutActivating);
}

void CensorOverlay::showEvent(QShowEvent *event) {
	QWidget::showEvent(event);
	applyX11OverlayHints();
}

void CensorOverlay::applyX11OverlayHints() {
#ifdef SENSORGUARD_HAS_X11
	Display *display = XOpenDisplay(nullptr);
	if (!display)
		return;

	const Window window = static_cast<Window>(winId());

	// Force the X server itself to route all pointer input to windows below us,
	// independent of Qt/window-manager mouse-transparency quirks.
	XRectangle emptyRectangle{};
	XShapeCombineRectangles(display, window, ShapeInput, 0, 0, &emptyRectangle, 0, ShapeSet, 0);

	// Stay visible above everything on every workspace without appearing in taskbars/pagers.
	const Atom wmState = XInternAtom(display, "_NET_WM_STATE", False);
	const Atom sticky = XInternAtom(display, "_NET_WM_STATE_STICKY", False);
	const Atom above = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);
	const Atom skipTaskbar = XInternAtom(display, "_NET_WM_STATE_SKIP_TASKBAR", False);
	const Atom skipPager = XInternAtom(display, "_NET_WM_STATE_SKIP_PAGER", False);
	Atom states[] = {sticky, above, skipTaskbar, skipPager};
	XChangeProperty(display, window, wmState, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char *>(states), 4);

	const Atom wmDesktop = XInternAtom(display, "_NET_WM_DESKTOP", False);
	const long allDesktops = -1;
	XChangeProperty(display, window, wmDesktop, XA_CARDINAL, 32, PropModeReplace,
		reinterpret_cast<const unsigned char *>(&allDesktops), 1);

	XFlush(display);
	XCloseDisplay(display);
#endif
}

void CensorOverlay::setDetections(const std::vector<DetectionResult> &detections) {
	m_detections = detections;
	update();
}

void CensorOverlay::setSourceSize(const QSize &size) {
	m_sourceSize = size;
	update();
}

void CensorOverlay::clearDetections() {
	if (m_detections.empty())
		return;

	m_detections.clear();
	update();
}

void CensorOverlay::paintEvent(QPaintEvent *event) {
	Q_UNUSED(event);

	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::black);

	for (DetectionResult &detection : m_detections) {
		const BoundingBox &box = detection.box;
		const double scaleX = m_sourceSize.width() > 0
			? static_cast<double>(width()) / m_sourceSize.width()
			: 1.0;
		const double scaleY = m_sourceSize.height() > 0
			? static_cast<double>(height()) / m_sourceSize.height()
			: 1.0;
		const QRect rectangle(
			static_cast<int>(box.x1 * scaleX),
			static_cast<int>(box.y1 * scaleY),
			static_cast<int>((box.x2 - box.x1) * scaleX),
			static_cast<int>((box.y2 - box.y1) * scaleY));
		if (rectangle.isValid())
			painter.drawRect(rectangle);
	}
}
