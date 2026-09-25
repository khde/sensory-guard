#include "SettingsTab.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QSlider>
#include <QSignalBlocker>
#include <QtGlobal>

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

    QGroupBox *performanceSettings = new QGroupBox("Performance", this);
    QVBoxLayout *performanceLayout = new QVBoxLayout(performanceSettings);

    m_ignoreSmallScreenChangesCheckBox = new QCheckBox("Ignore small screen changes", performanceSettings);
    m_ignoreSmallScreenChangesCheckBox->setChecked(true);
    performanceLayout->addWidget(m_ignoreSmallScreenChangesCheckBox);

    m_frameChangeLabel = new QLabel(performanceSettings);
    performanceLayout->addWidget(m_frameChangeLabel);

    m_frameChangeSlider = new QSlider(Qt::Horizontal, performanceSettings);
    m_frameChangeSlider->setRange(0, 100);
    m_frameChangeSlider->setValue(15);
    m_frameChangeSlider->setSingleStep(1);
    performanceLayout->addWidget(m_frameChangeSlider);

    layout->addWidget(performanceSettings);

    connect(m_minimizeToTrayCheckBox, &QCheckBox::toggled, this, [this](bool) {
        emit settingsChanged(
            m_minimizeToTrayCheckBox->isChecked(),
            m_ignoreSmallScreenChangesCheckBox->isChecked(),
            m_frameChangeSlider->value() * 0.001f);
    });

    connect(m_ignoreSmallScreenChangesCheckBox, &QCheckBox::toggled, this, [this](bool) {
        updateFrameChangeControls();
        emit settingsChanged(
            m_minimizeToTrayCheckBox->isChecked(),
            m_ignoreSmallScreenChangesCheckBox->isChecked(),
            m_frameChangeSlider->value() * 0.001f);
    });

    connect(m_frameChangeSlider, &QSlider::valueChanged, this, [this](int value) {
        updateFrameChangeLabel(value);
        emit settingsChanged(
            m_minimizeToTrayCheckBox->isChecked(),
            m_ignoreSmallScreenChangesCheckBox->isChecked(),
            value * 0.001f);
    });

    updateFrameChangeLabel(m_frameChangeSlider->value());
    updateFrameChangeControls();

    layout->addStretch();
}

void SettingsTab::setSettings(const UserSettings &settings) {
    const QSignalBlocker minimizeBlocker(m_minimizeToTrayCheckBox);
    const QSignalBlocker ignoreBlocker(m_ignoreSmallScreenChangesCheckBox);
    const QSignalBlocker sliderBlocker(m_frameChangeSlider);
    m_minimizeToTrayCheckBox->setChecked(settings.minimizeToTray);
    m_ignoreSmallScreenChangesCheckBox->setChecked(settings.ignoreSmallScreenChanges);
    m_frameChangeSlider->setValue(qBound(0, qRound(settings.frameChangeThreshold / 0.001f), 100));
    updateFrameChangeLabel(m_frameChangeSlider->value());
    updateFrameChangeControls();
}

void SettingsTab::updateFrameChangeLabel(int value) {
    m_frameChangeLabel->setText(
        QString("Change tolerance: %1%").arg(value * 0.1, 0, 'f', 1));
}

void SettingsTab::updateFrameChangeControls() {
    m_frameChangeSlider->setEnabled(m_ignoreSmallScreenChangesCheckBox->isChecked());
    m_frameChangeLabel->setEnabled(m_ignoreSmallScreenChangesCheckBox->isChecked());
}
