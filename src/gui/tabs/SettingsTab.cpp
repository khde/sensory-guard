#include "SettingsTab.h"

#include <QLabel>
#include <QVBoxLayout>

SettingsTab::SettingsTab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("SYSTEM SETTINGS", this);
    layout->addWidget(title);

    layout->addStretch();
}