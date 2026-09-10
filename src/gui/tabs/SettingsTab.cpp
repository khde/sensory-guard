#include "SettingsTab.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>

SettingsTab::SettingsTab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("SYSTEM SETTINGS", this);
    layout->addWidget(title);

    QGroupBox *generalSettings = new QGroupBox("General Settings", this);
    QVBoxLayout *generalLayout = new QVBoxLayout(generalSettings);

    m_minimizeToTrayCheckBox = new QCheckBox("Minimize to tray on close", generalSettings);
    m_minimizeToTrayCheckBox->setChecked(true);
    generalLayout->addWidget(m_minimizeToTrayCheckBox);

    QCheckBox *launchOnStartupCheckBox = new QCheckBox("Launch on startup", generalSettings);
    generalLayout->addWidget(launchOnStartupCheckBox);
    layout->addWidget(generalSettings);

    connect(m_minimizeToTrayCheckBox, &QCheckBox::toggled, this, &SettingsTab::minimizeToTrayChanged);

    layout->addStretch();
}

bool SettingsTab::minimizeToTrayEnabled() const
{
    return m_minimizeToTrayCheckBox->isChecked();
}