#include "SettingsTab.h"

#include "inference/HardwareDevices.h"
#include "platform/DisplayDevices.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QSignalBlocker>
#include <QMessageBox>
#include <QPushButton>
#include <QtGlobal>
#include <QSpinBox>
#include <QScrollArea>
#include <QFrame>
#include <QDialog>
#include <QDialogButtonBox>
#include <QKeySequenceEdit>

SettingsTab::SettingsTab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("SYSTEM SETTINGS", this);
    title->setProperty("role", "pageTitle");
    layout->addWidget(title);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);
    QWidget *contentWidget = new QWidget(scrollArea);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 8, 0);
    scrollArea->setWidget(contentWidget);
    layout->addWidget(scrollArea);

    QGroupBox *generalSettings = new QGroupBox("General Settings", contentWidget);
    QVBoxLayout *generalLayout = new QVBoxLayout(generalSettings);

    m_minimizeToTrayCheckBox = new QCheckBox("Minimize to tray on close", generalSettings);
    m_minimizeToTrayCheckBox->setChecked(false);
    generalLayout->addWidget(m_minimizeToTrayCheckBox);

    // Not implemented
    // QCheckBox *launchOnStartupCheckBox = new QCheckBox("Launch on startup", generalSettings);
    // generalLayout->addWidget(launchOnStartupCheckBox);

    contentLayout->addWidget(generalSettings);

    QGroupBox *hardwareSettings = new QGroupBox("Hardware", contentWidget);
    QVBoxLayout *hardwareLayout = new QVBoxLayout(hardwareSettings);

    hardwareLayout->addWidget(new QLabel("Monitor", hardwareSettings));
    m_displayComboBox = new QComboBox(hardwareSettings);
    const std::vector<DisplayDescriptor> displays = enumerateDisplays();
    for (const DisplayDescriptor &display : displays) {
        m_displayComboBox->addItem(
            QString::fromStdString(display.name) + " - " + QString::number(display.desktopWidth) + "x" + QString::number(display.desktopHeight),
            QString::fromStdString(display.id));
    }
    if (displays.empty()) {
        m_displayComboBox->addItem("No compatible monitor detected");
        m_displayComboBox->setEnabled(false);
    }
    hardwareLayout->addWidget(m_displayComboBox);

    hardwareLayout->addWidget(new QLabel("Inference backend", hardwareSettings));
    m_backendComboBox = new QComboBox(hardwareSettings);
    m_backendComboBox->addItem("CPU", static_cast<int>(InferenceBackend::Cpu));
#ifdef _WIN32
    m_backendComboBox->addItem("GPU (DirectML)", static_cast<int>(InferenceBackend::DirectML));
#endif
    hardwareLayout->addWidget(m_backendComboBox);

    m_gpuLabel = new QLabel("GPU device", hardwareSettings);
    hardwareLayout->addWidget(m_gpuLabel);
    m_gpuComboBox = new QComboBox(hardwareSettings);
    const std::vector<HardwareDeviceInfo> gpuDevices = enumerateDirectMLDevices();
    for (const HardwareDeviceInfo &device : gpuDevices) {
        m_gpuComboBox->addItem(QString::fromStdString(device.name), device.index);
        m_gpuComboBox->setItemData(
            m_gpuComboBox->count() - 1,
            QString::fromStdString(device.id),
            Qt::UserRole + 1);
    }
    if (gpuDevices.empty()) {
        m_gpuComboBox->addItem("No DirectML GPU detected", -1);
    }
    hardwareLayout->addWidget(m_gpuComboBox);

    m_cpuThreadLabel = new QLabel("CPU inference threads", hardwareSettings);
    hardwareLayout->addWidget(m_cpuThreadLabel);
    m_cpuThreadSpinBox = new QSpinBox(hardwareSettings);
    m_cpuThreadSpinBox->setRange(1, 64);
    m_cpuThreadSpinBox->setValue(4);
    hardwareLayout->addWidget(m_cpuThreadSpinBox);

    contentLayout->addWidget(hardwareSettings);

    QGroupBox *appearanceSettings = new QGroupBox("Appearance", contentWidget);
    QVBoxLayout *appearanceLayout = new QVBoxLayout(appearanceSettings);
    appearanceLayout->addWidget(new QLabel("Theme", appearanceSettings));
    m_themeComboBox = new QComboBox(appearanceSettings);
    m_themeComboBox->addItem("System", static_cast<int>(UserSettings::Theme::System));
    m_themeComboBox->addItem("Light", static_cast<int>(UserSettings::Theme::Light));
    m_themeComboBox->addItem("Dark", static_cast<int>(UserSettings::Theme::Dark));
    appearanceLayout->addWidget(m_themeComboBox);
    contentLayout->addWidget(appearanceSettings);

    QGroupBox *toggleHotkeySettings = new QGroupBox("Toggle shortcut", contentWidget);
    QVBoxLayout *toggleHotkeyLayout = new QVBoxLayout(toggleHotkeySettings);
    m_toggleHotkeyCheckBox = new QCheckBox("Enable toggle shortcut", toggleHotkeySettings);
    m_toggleHotkeyCheckBox->setChecked(true);
    toggleHotkeyLayout->addWidget(m_toggleHotkeyCheckBox);
    m_toggleHotkeyButton = new QPushButton("Set toggle shortcut: Ctrl+Alt+Shift+S", toggleHotkeySettings);
    toggleHotkeyLayout->addWidget(m_toggleHotkeyButton);
    contentLayout->addWidget(toggleHotkeySettings);

    QGroupBox *performanceSettings = new QGroupBox("Performance", contentWidget);
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

    contentLayout->addWidget(performanceSettings);

    QPushButton *resetButton = new QPushButton("Reset settings to default", contentWidget);
    contentLayout->addWidget(resetButton);
    connect(resetButton, &QPushButton::clicked, this, [this] {
        const QMessageBox::StandardButton choice = QMessageBox::question(
            this,
            "Reset Settings",
            "Reset all settings to their defaults?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (choice == QMessageBox::Yes) {
            emit resetRequested();
        }
    });

    connect(m_minimizeToTrayCheckBox, &QCheckBox::toggled, this, [this](bool) {emitCurrentSettings();});
    connect(m_ignoreSmallScreenChangesCheckBox, &QCheckBox::toggled, this, [this](bool) {updateFrameChangeControls(); emitCurrentSettings();});
    connect(m_frameChangeSlider, &QSlider::valueChanged, this, [this](int value) {updateFrameChangeLabel(value); emitCurrentSettings();});
    connect(m_themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {emitCurrentSettings();});
    connect(m_backendComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {updateFrameChangeControls(); emitCurrentSettings();});
    connect(m_gpuComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {emitCurrentSettings();});
    connect(m_cpuThreadSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int) {emitCurrentSettings();});
    connect(m_displayComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {emitCurrentSettings();});
    connect(m_toggleHotkeyCheckBox, &QCheckBox::toggled, this, [this](bool) {emitCurrentSettings();});
    connect(m_toggleHotkeyButton, &QPushButton::clicked, this, [this] {
        QDialog dialog(this);
        dialog.setWindowTitle("Set global toggle shortcut");
        QVBoxLayout dialogLayout(&dialog);
        dialogLayout.addWidget(new QLabel("Press the shortcut you want to use.", &dialog));

        QKeySequenceEdit editor(QKeySequence::fromString(m_toggleHotkey, QKeySequence::PortableText), &dialog);
        editor.setMaximumSequenceLength(1);
        dialogLayout.addWidget(&editor);

        QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        dialogLayout.addWidget(&buttons);
        connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted && !editor.keySequence().isEmpty()) {
            m_toggleHotkey = editor.keySequence().toString(QKeySequence::PortableText);
            m_toggleHotkeyButton->setText("Set toggle shortcut: " + m_toggleHotkey);
            emitCurrentSettings();
        }
    });

    updateFrameChangeLabel(m_frameChangeSlider->value());
    updateFrameChangeControls();

    contentLayout->addStretch();
}

void SettingsTab::setSettings(const UserSettings &settings) {
    const QSignalBlocker minimizeBlocker(m_minimizeToTrayCheckBox);
    const QSignalBlocker ignoreBlocker(m_ignoreSmallScreenChangesCheckBox);
    const QSignalBlocker sliderBlocker(m_frameChangeSlider);
    const QSignalBlocker themeBlocker(m_themeComboBox);
    const QSignalBlocker backendBlocker(m_backendComboBox);
    const QSignalBlocker gpuBlocker(m_gpuComboBox);
    const QSignalBlocker threadBlocker(m_cpuThreadSpinBox);
    const QSignalBlocker displayBlocker(m_displayComboBox);
    const QSignalBlocker toggleHotkeyBlocker(m_toggleHotkeyCheckBox);
    m_minimizeToTrayCheckBox->setChecked(settings.minimizeToTray);
    m_ignoreSmallScreenChangesCheckBox->setChecked(settings.ignoreSmallScreenChanges);
    m_frameChangeSlider->setValue(qBound(0, qRound(settings.frameChangeThreshold / 0.001f), 100));
    const int themeIndex = m_themeComboBox->findData(static_cast<int>(settings.theme));
    if (themeIndex >= 0) {
        m_themeComboBox->setCurrentIndex(themeIndex);
    }
    const int backendIndex = m_backendComboBox->findData(static_cast<int>(settings.hardware.backend));
    if (backendIndex >= 0) {
        m_backendComboBox->setCurrentIndex(backendIndex);
    }
    int gpuIndex = -1;
    if (!settings.hardware.deviceId.empty()) {
        gpuIndex = m_gpuComboBox->findData(
            QString::fromStdString(settings.hardware.deviceId),
            Qt::UserRole + 1);
    }
    if (gpuIndex < 0){
        gpuIndex = m_gpuComboBox->findData(settings.hardware.deviceIndex);
    }
    m_gpuComboBox->setCurrentIndex(gpuIndex >= 0 ? gpuIndex : 0);
    m_cpuThreadSpinBox->setValue(qBound(1, settings.hardware.cpuThreadCount, 64));
    int displayIndex = m_displayComboBox->findData(QString::fromStdString(settings.display.displayId));
    if (displayIndex < 0 && !settings.display.displayId.empty()) {
        m_displayComboBox->addItem(
            "Unavailable monitor - " + QString::fromStdString(settings.display.displayId), QString::fromStdString(settings.display.displayId));
        displayIndex = m_displayComboBox->count() - 1;
    }
    if (displayIndex < 0 && settings.display.displayId.empty() && m_displayComboBox->count() > 0) {
        const std::vector<DisplayDescriptor> displays = enumerateDisplays();
        for (int index = 0; index < static_cast<int>(displays.size()); ++index) {
            if (displays[static_cast<size_t>(index)].desktopX == 0 &&
                displays[static_cast<size_t>(index)].desktopY == 0) {
                displayIndex = index;
                break;
            }
        }
        if (displayIndex < 0)
            displayIndex = 0;
    }
    if (displayIndex >= 0)
        m_displayComboBox->setCurrentIndex(displayIndex);
    m_toggleHotkeyCheckBox->setChecked(settings.toggleHotkeyEnabled);
    m_toggleHotkey = settings.toggleHotkey;
    m_toggleHotkeyButton->setText("Set toggle shortcut: " + m_toggleHotkey);
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
    const bool directMlSelected = m_backendComboBox->currentData().toInt() == static_cast<int>(InferenceBackend::DirectML);
    m_gpuComboBox->setEnabled(directMlSelected && m_gpuComboBox->currentData().toInt() >= 0);
    m_gpuLabel->setVisible(directMlSelected);
    m_gpuComboBox->setVisible(directMlSelected);
    m_cpuThreadLabel->setVisible(!directMlSelected);
    m_cpuThreadSpinBox->setVisible(!directMlSelected);
    m_cpuThreadSpinBox->setEnabled(!directMlSelected);
}

void SettingsTab::emitCurrentSettings() {
    emit settingsChanged(
        m_minimizeToTrayCheckBox->isChecked(),
        m_ignoreSmallScreenChangesCheckBox->isChecked(),
        m_frameChangeSlider->value() * 0.001f,
        static_cast<UserSettings::Theme>(m_themeComboBox->currentData().toInt()),
        static_cast<InferenceBackend>(m_backendComboBox->currentData().toInt()),
        m_gpuComboBox->currentData().toInt(),
        m_gpuComboBox->currentData(Qt::UserRole + 1).toString(),
        m_cpuThreadSpinBox->value(),
        m_displayComboBox->currentData().toString(),
        m_toggleHotkeyCheckBox->isChecked(),
        m_toggleHotkey);
}
