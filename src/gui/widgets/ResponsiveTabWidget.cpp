#include "ResponsiveTabWidget.h"

#include <QTabBar>

namespace {
class ResponsiveTabBar : public QTabBar {
    QSize sizeHint() const override {
        QSize size = QTabBar::sizeHint();
        if (parentWidget()) {
            size.setWidth(parentWidget()->width());
        }
        return size;
    }

protected:
    QSize tabSizeHint(int index) const override {
        QSize size = QTabBar::tabSizeHint(index);
        if (count() > 0 && width() > 0) {
            size.setWidth(width() / count());
        }
        return size;
    }
};
}

ResponsiveTabWidget::ResponsiveTabWidget(QWidget *parent) : QTabWidget(parent) {
    setTabBar(new ResponsiveTabBar());
}
