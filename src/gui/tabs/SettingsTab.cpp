#include "SettingsTab.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QSignalBlocker>

SettingsTab::SettingsTab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("SYSTEM SETTINGS", this);
    layout->addWidget(title);

    QGroupBox *generalSettings = new QGroupBox("General Settings", this);
    QVBoxLayout *generalLayout = new QVBoxLayout(generalSettings);

    m_minimizeToTrayCheckBox = new QCheckBox("Minimize to tray on close", generalSettings);
    m_minimizeToTrayCheckBox->setChecked(true);
    generalLayout->addWidget(m_minimizeToTrayCheckBox);

    // Not implemented
    // QCheckBox *launchOnStartupCheckBox = new QCheckBox("Launch on startup", generalSettings);
    // generalLayout->addWidget(launchOnStartupCheckBox);

    layout->addWidget(generalSettings);

    connect(m_minimizeToTrayCheckBox, &QCheckBox::toggled, this, [this](bool) {
        emit settingsChanged(m_minimizeToTrayCheckBox->isChecked());
    });

    layout->addStretch();
}

void SettingsTab::setSettings(const UserSettings &settings)
{
    const QSignalBlocker blocker(this);
    m_minimizeToTrayCheckBox->setChecked(settings.minimizeToTray);
}
