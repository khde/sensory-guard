#include "DetectionTab.h"

#include <QLabel>
#include <QStringList>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QSlider>

DetectionTab::DetectionTab(QWidget *parent): QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("AI FILTERS", this);
    layout->addWidget(title);

    sensitivityLabel = new QLabel("Sensitivity: 20%", this);
    sensitivitySlider = new QSlider(Qt::Horizontal, this);
    sensitivitySlider->setRange(1, 100);
    sensitivitySlider->setValue(20);

    layout->addWidget(sensitivityLabel);
    layout->addWidget(sensitivitySlider);

    connect(
        sensitivitySlider,
        &QSlider::valueChanged,
        this,
        &DetectionTab::updateSensitivityLabel
    );
    connect(
        sensitivitySlider,
        &QSlider::valueChanged,
        this,
        [this](int) { emitSettingsChanged(); }
    );

    fpsLabel = new QLabel("Max FPS: 20", this);
    fpsSlider = new QSlider(Qt::Horizontal, this);
    fpsSlider->setRange(1, 60);
    fpsSlider->setValue(20);

    layout->addWidget(fpsLabel);
    layout->addWidget(fpsSlider);

    connect(fpsSlider, &QSlider::valueChanged, this, &DetectionTab::updateFpsLabel);
    connect(fpsSlider, &QSlider::valueChanged, this, [this](int) {emitSettingsChanged();});

    QStringList exposedLabels = {
        "FEMALE_GENITALIA_EXPOSED",
        "FEMALE_BREAST_EXPOSED",
        "BUTTOCKS_EXPOSED",
        "MALE_GENITALIA_EXPOSED",
        "MALE_BREAST_EXPOSED",
        "ANUS_EXPOSED",
        "BELLY_EXPOSED",
        "ARMPITS_EXPOSED",
        "FEET_EXPOSED"
    };

    QGroupBox *exposedGroup = new QGroupBox("Exposed", this);
    QVBoxLayout *exposedLayout = new QVBoxLayout(exposedGroup);
    for (QString &label : exposedLabels)
        addLabelCheckbox(label, true, exposedLayout, exposedGroup);
    layout->addWidget(exposedGroup);

    QStringList coveredLabels = {
        "FEMALE_GENITALIA_COVERED",
        "FEMALE_BREAST_COVERED",
        "BUTTOCKS_COVERED",
        "ANUS_COVERED",
        "BELLY_COVERED",
        "ARMPITS_COVERED",
        "FEET_COVERED",
        "FACE_FEMALE",
        "FACE_MALE"
    };

    QGroupBox *coveredGroup = new QGroupBox("Covered", this);
    QVBoxLayout *coveredLayout = new QVBoxLayout(coveredGroup);
    for (QString &label : coveredLabels)
        addLabelCheckbox(label, false, coveredLayout, coveredGroup);
    layout->addWidget(coveredGroup);

    layout->addStretch();
}

QCheckBox *DetectionTab::addLabelCheckbox(const QString &label, bool checked, QVBoxLayout *layout, QWidget *parent) {
    QCheckBox *checkBox = new QCheckBox(label, parent);
    checkBox->setChecked(checked);
    layout->addWidget(checkBox);
    m_labelCheckBoxes.append(checkBox);
    connect(checkBox, &QCheckBox::toggled, this, [this](bool) { emitSettingsChanged(); });
    return checkBox;
}

void DetectionTab::emitSettingsChanged() {
    QStringList enabledLabels;
    for (const QCheckBox *checkBox : m_labelCheckBoxes) {
        if (checkBox->isChecked())
            enabledLabels.append(checkBox->text());
    }

    const float confidenceThreshold = sensitivitySlider->value() / 100.0f;
    const int maxFps = fpsSlider->value();
    emit settingsChanged(confidenceThreshold, enabledLabels, maxFps);
}

void DetectionTab::broadcastCurrentSettings() {
    emitSettingsChanged();
}

void DetectionTab::updateSensitivityLabel(int value) {
    sensitivityLabel->setText(
        "Sensitivity: " + QString::number(value) + "%"
    );
}

void DetectionTab::updateFpsLabel(int value) {
    fpsLabel->setText("Max FPS: " + QString::number(value));
}