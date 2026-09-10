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

    sensitivityLabel = new QLabel("Sensitivity: 60%", this);
    sensitivitySlider = new QSlider(Qt::Horizontal, this);
    sensitivitySlider->setRange(0, 100);
    sensitivitySlider->setValue(60);

    layout->addWidget(sensitivityLabel);
    layout->addWidget(sensitivitySlider);

    connect(
        sensitivitySlider,
        &QSlider::valueChanged,
        this,
        &DetectionTab::updateSensitivityLabel
    );
        
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
    for (const QString &label : exposedLabels)
    {
        QCheckBox *checkBox = new QCheckBox(label, exposedGroup);
        checkBox->setChecked(true);
        exposedLayout->addWidget(checkBox);
    }
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
    for (const QString &label : coveredLabels)
    {
        QCheckBox *checkBox = new QCheckBox(label, coveredGroup);
        checkBox->setChecked(false);
        coveredLayout->addWidget(checkBox);
    }
    layout->addWidget(coveredGroup);

    layout->addStretch();
}

void DetectionTab::updateSensitivityLabel(int value) {
    sensitivityLabel->setText(
        "Sensitivity: " + QString::number(value) + "%"
    );
}