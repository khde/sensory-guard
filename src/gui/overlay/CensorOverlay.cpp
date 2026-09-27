#include "CensorOverlay.h"
#include "CensorStyle.h"

#include <QPainter>
#include <QPaintEvent>
#include <QShowEvent>
#include <QString>
#include <QImage>
#include <opencv2/imgproc.hpp>

#ifdef SENSORGUARD_HAS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

CensorOverlay::CensorOverlay(QWidget *parent): QWidget(parent), m_censorStyle(std::make_unique<CensorStyle>()) {
	setWindowFlags(Qt::Tool |
				   Qt::FramelessWindowHint |
				   Qt::WindowStaysOnTopHint |
				   Qt::WindowDoesNotAcceptFocus |
				   Qt::WindowTransparentForInput |
				   Qt::X11BypassWindowManagerHint);
	setAttribute(Qt::WA_NativeWindow);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setAttribute(Qt::WA_ShowWithoutActivating);
}

CensorOverlay::~CensorOverlay() = default;

void CensorOverlay::showEvent(QShowEvent *event) {
	QWidget::showEvent(event);
	applyX11OverlayHints();
#ifdef _WIN32
	SetWindowDisplayAffinity(reinterpret_cast<HWND>(winId()), WDA_EXCLUDEFROMCAPTURE);
#endif
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

void CensorOverlay::setSourceFrame(const cv::Mat &frame) {
	m_sourceFrame = frame.clone();
	update();
}

void CensorOverlay::setCensoringConfig(const CensoringConfig& config) {
	m_censorStyle->setConfig(config);
	m_scaleFactor = config.scaleFactor;
	update();
}

void CensorOverlay::clearDetections() {
	if (m_detections.empty())
		return;

	m_detections.clear();
	update();
}

BoundingBox CensorOverlay::scaleBoundingBox(const BoundingBox &box, float factor) const {
	if (factor <= 0.0f)
		factor = 1.0f;

	// Calculate dimensions
	int width = box.x2 - box.x1;
	int height = box.y2 - box.y1;
	int centerX = box.x1 + width / 2;
	int centerY = box.y1 + height / 2;

	// Scale dimensions
	int scaledWidth = static_cast<int>(width * factor);
	int scaledHeight = static_cast<int>(height * factor);

	// Center-scale: expand from center point
	BoundingBox scaledBox;
	scaledBox.x1 = centerX - scaledWidth / 2;
	scaledBox.y1 = centerY - scaledHeight / 2;
	scaledBox.x2 = centerX + scaledWidth / 2;
	scaledBox.y2 = centerY + scaledHeight / 2;

	return scaledBox;
}

void CensorOverlay::paintBlackCensoringWithQt(QPainter &painter) {
	// Use Qt painting for black boxes, requiring no frame data
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::black);

	for (const DetectionResult &detection : m_detections) {
		BoundingBox box = detection.box;

		if (std::abs(m_scaleFactor - 1.0f) > 0.001f) // Avoid unnecessary scaling when factor is close to 1.0
			box = scaleBoundingBox(box, m_scaleFactor);

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
		if (rectangle.isValid()) {
			painter.fillRect(rectangle, Qt::black);
		}
	}
}

void CensorOverlay::paintFrameWithCensoring(QPainter &painter) {
	// Only paint detected regions, not entire frame!
	if (m_sourceFrame.empty())
		return;

	const double scaleX = width() > 0 ? static_cast<double>(width()) / m_sourceSize.width() : 1.0;
	const double scaleY = height() > 0 ? static_cast<double>(height()) / m_sourceSize.height() : 1.0;

	// Process and paint only the detected regions
	for (const DetectionResult &detection : m_detections) {
		BoundingBox box = detection.box;

		if (std::abs(m_scaleFactor - 1.0f) > 0.001f)
			box = scaleBoundingBox(box, m_scaleFactor);

		// Bounds check
		if (box.x1 < 0 || box.y1 < 0 || box.x2 > m_sourceFrame.cols || box.y2 > m_sourceFrame.rows)
			continue;

		// Extract region from source frame
		cv::Rect roi(box.x1, box.y1, box.x2 - box.x1, box.y2 - box.y1);
		cv::Mat region = m_sourceFrame(roi).clone();

		// Apply censoring using CensorStyle
		m_censorStyle->applyCensoring(region);

		// Convert region BGR → RGB
		cv::Mat rgb;
		cv::cvtColor(region, rgb, cv::COLOR_BGR2RGB);

		// Create QImage from region
		QImage img(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);

		// Scale if necessary
		QImage displayImg = img;
		if (std::abs(scaleX - 1.0) > 0.001 || std::abs(scaleY - 1.0) > 0.001) {
			displayImg = img.scaledToWidth(static_cast<int>((box.x2 - box.x1) * scaleX), Qt::FastTransformation);
		}

		// Paint only this region at the correct position
		int screenX = static_cast<int>(box.x1 * scaleX);
		int screenY = static_cast<int>(box.y1 * scaleY);
		painter.drawImage(screenX, screenY, displayImg);
	}
}

void CensorOverlay::paintEvent(QPaintEvent *event) {
	Q_UNUSED(event);

	if (m_detections.empty())
		return;

	QPainter painter(this);

	if (m_censorStyle->getStyle() == CensoringStyle::Black) {
		// Black censoring: no frame data needed
		paintBlackCensoringWithQt(painter);
	} else {
		// Blur/Pixelation: use frame-based rendering
		paintFrameWithCensoring(painter);
	}
}

