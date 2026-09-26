#include "SettingsTab.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QSignalBlocker>
#include <QtGlobal>

SettingsTab::SettingsTab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("SYSTEM SETTINGS", this);
    title->setProperty("role", "pageTitle");
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

    QGroupBox *appearanceSettings = new QGroupBox("Appearance", this);
    QVBoxLayout *appearanceLayout = new QVBoxLayout(appearanceSettings);
    appearanceLayout->addWidget(new QLabel("Theme", appearanceSettings));
    m_themeComboBox = new QComboBox(appearanceSettings);
    m_themeComboBox->addItem("System", static_cast<int>(UserSettings::Theme::System));
    m_themeComboBox->addItem("Light", static_cast<int>(UserSettings::Theme::Light));
    m_themeComboBox->addItem("Dark", static_cast<int>(UserSettings::Theme::Dark));
    appearanceLayout->addWidget(m_themeComboBox);
    layout->addWidget(appearanceSettings);

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
            m_frameChangeSlider->value() * 0.001f,
            static_cast<UserSettings::Theme>(m_themeComboBox->currentData().toInt()));
    });

    connect(m_ignoreSmallScreenChangesCheckBox, &QCheckBox::toggled, this, [this](bool) {
        updateFrameChangeControls();
        emit settingsChanged(
            m_minimizeToTrayCheckBox->isChecked(),
            m_ignoreSmallScreenChangesCheckBox->isChecked(),
            m_frameChangeSlider->value() * 0.001f,
            static_cast<UserSettings::Theme>(m_themeComboBox->currentData().toInt()));
    });

    connect(m_frameChangeSlider, &QSlider::valueChanged, this, [this](int value) {
        updateFrameChangeLabel(value);
        emit settingsChanged(
            m_minimizeToTrayCheckBox->isChecked(),
            m_ignoreSmallScreenChangesCheckBox->isChecked(),
                value * 0.001f,
                static_cast<UserSettings::Theme>(m_themeComboBox->currentData().toInt()));
    });

    connect(m_themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
    emit settingsChanged(
        m_minimizeToTrayCheckBox->isChecked(),
        m_ignoreSmallScreenChangesCheckBox->isChecked(),
        m_frameChangeSlider->value() * 0.001f,
        static_cast<UserSettings::Theme>(m_themeComboBox->currentData().toInt()));
    });

    updateFrameChangeLabel(m_frameChangeSlider->value());
    updateFrameChangeControls();

    layout->addStretch();
}

void SettingsTab::setSettings(const UserSettings &settings) {
    const QSignalBlocker minimizeBlocker(m_minimizeToTrayCheckBox);
    const QSignalBlocker ignoreBlocker(m_ignoreSmallScreenChangesCheckBox);
    const QSignalBlocker sliderBlocker(m_frameChangeSlider);
    const QSignalBlocker themeBlocker(m_themeComboBox);
    m_minimizeToTrayCheckBox->setChecked(settings.minimizeToTray);
    m_ignoreSmallScreenChangesCheckBox->setChecked(settings.ignoreSmallScreenChanges);
    m_frameChangeSlider->setValue(qBound(0, qRound(settings.frameChangeThreshold / 0.001f), 100));
    const int themeIndex = m_themeComboBox->findData(static_cast<int>(settings.theme));
    if (themeIndex >= 0) {
        m_themeComboBox->setCurrentIndex(themeIndex);
    }
    updateFrameChangeLabel(m_frameChangeSlider->value());
    updateFrameChangeControls();
}

void SettingsTab::updateFrameChangeLabel(int value) {
    m_frameChangeLabel->setText(
        QString("Change tolerance: %1%").arg(value * 0.1, 0, 'f', 1));
}

void SettingsTab::updateFrameChangeControls() {
    const bool enabled = m_ignoreSmallScreenChangesCheckBox->isChecked();
    m_frameChangeSlider->setEnabled(enabled);
    m_frameChangeLabel->setEnabled(enabled);
}
