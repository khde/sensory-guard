#include "DetectionTab.h"

#include <QLabel>
#include <QStringList>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QSlider>
#include <QMap>
#include <QSignalBlocker>

// Mapping from display names to model label names
static const QMap<QString, QString> LABEL_MAPPING = {
    {"Exposed Female Genitalia", "FEMALE_GENITALIA_EXPOSED"},
    {"Covered Female Genitalia", "FEMALE_GENITALIA_COVERED"},
    {"Exposed Female Breast", "FEMALE_BREAST_EXPOSED"},
    {"Covered Female Breast", "FEMALE_BREAST_COVERED"},
    {"Exposed Buttocks", "BUTTOCKS_EXPOSED"},
    {"Covered Buttocks", "BUTTOCKS_COVERED"},
    {"Exposed Male Genitalia", "MALE_GENITALIA_EXPOSED"},
    {"Exposed Male Breast", "MALE_BREAST_EXPOSED"},
    {"Exposed Anus", "ANUS_EXPOSED"},
    {"Covered Anus", "ANUS_COVERED"},
    {"Exposed Belly", "BELLY_EXPOSED"},
    {"Covered Belly", "BELLY_COVERED"},
    {"Exposed Armpits", "ARMPITS_EXPOSED"},
    {"Covered Armpits", "ARMPITS_COVERED"},
    {"Exposed Feet", "FEET_EXPOSED"},
    {"Covered Feet", "FEET_COVERED"},
    {"Female Face", "FACE_FEMALE"},
    {"Male Face", "FACE_MALE"}
};

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

    connect( sensitivitySlider, &QSlider::valueChanged, this,&DetectionTab::updateSensitivityLabel);
    connect(sensitivitySlider, &QSlider::valueChanged, this, [this](int) { emitSettingsChanged();});

    fpsLabel = new QLabel("Max FPS: 12", this);
    fpsSlider = new QSlider(Qt::Horizontal, this);
    fpsSlider->setRange(1, 60);
    fpsSlider->setValue(12);

    layout->addWidget(fpsLabel);
    layout->addWidget(fpsSlider);

    connect(fpsSlider, &QSlider::valueChanged, this, &DetectionTab::updateFpsLabel);
    connect(fpsSlider, &QSlider::valueChanged, this, [this](int) {emitSettingsChanged();});

    QStringList exposedLabels = {
        "Exposed Female Genitalia",
        "Exposed Female Breast",
        "Exposed Buttocks",
        "Exposed Male Genitalia",
        "Exposed Male Breast",
        "Exposed Anus",
        "Exposed Belly",
        "Exposed Armpits",
        "Exposed Feet"
    };

    QGroupBox *exposedGroup = new QGroupBox("Exposed Categories", this);
    QVBoxLayout *exposedLayout = new QVBoxLayout(exposedGroup);
    for (QString &label : exposedLabels)
        addLabelCheckbox(label, true, exposedLayout, exposedGroup);
    layout->addWidget(exposedGroup);

    QStringList coveredLabels = {
        "Covered Female Genitalia",
        "Covered Female Breast",
        "Covered Buttocks",
        "Covered Anus",
        "Covered Belly",
        "Covered Armpits",
        "Covered Feet",
        "Female Face",
        "Male Face"
    };

    QGroupBox *coveredGroup = new QGroupBox("Covered Categories", this);
    QVBoxLayout *coveredLayout = new QVBoxLayout(coveredGroup);
    for (QString &label : coveredLabels)
        addLabelCheckbox(label, false, coveredLayout, coveredGroup);
    layout->addWidget(coveredGroup);

    layout->addStretch();
}

void DetectionTab::setSettings(const UserSettings &settings) {
    const QSignalBlocker blocker(this);
    sensitivitySlider->setValue(qBound(1, qRound(settings.sensitivity * 100.0f), 100));
    fpsSlider->setValue(qBound(1, settings.maximumFps, 60));

    for (QCheckBox *checkBox : m_labelCheckBoxes) {
        const QString modelLabel = LABEL_MAPPING.value(checkBox->text());
        checkBox->setChecked(settings.enabledLabels.contains(modelLabel));
    }
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
        if (checkBox->isChecked()) {
            QString displayName = checkBox->text();
            if (LABEL_MAPPING.contains(displayName)) {
                enabledLabels.append(LABEL_MAPPING[displayName]);
            }
        }
    }
    emit settingsChanged(
        sensitivitySlider->value() / 100.0f,
        enabledLabels,
        fpsSlider->value());
}

void DetectionTab::updateSensitivityLabel(int value) {
    sensitivityLabel->setText(
        "Sensitivity: " + QString::number(value) + "%"
    );
}

void DetectionTab::updateFpsLabel(int value) {
    fpsLabel->setText("Max FPS: " + QString::number(value));
}