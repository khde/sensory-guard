#include "CustomizeTab.h"

#include <QLabel>
#include <QVBoxLayout>

CustomizeTab::CustomizeTab(QWidget *parent): QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("CENSOR STYLING", this);
    layout->addWidget(title);

    layout->addStretch();
}