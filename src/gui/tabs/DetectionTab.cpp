#include "DetectionTab.h"

#include <QLabel>
#include <QVBoxLayout>

DetectionTab::DetectionTab(QWidget *parent): QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("AI FILTERS", this);
    layout->addWidget(title);

    layout->addStretch();
}