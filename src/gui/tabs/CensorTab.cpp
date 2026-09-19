#include "CensorTab.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QSlider>

CensorTab::CensorTab(QWidget *parent): QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("CENSOR STYLING", this);
    layout->addWidget(title);

    QGroupBox *censoringGroup = new QGroupBox("Censoring Method", this);
    QVBoxLayout *censoringLayout = new QVBoxLayout(censoringGroup);

    QLabel *styleLabel = new QLabel("Censoring Style:", this);
    m_censoringStyleCombo = new QComboBox(this);
    m_censoringStyleCombo->addItem("Black Box", static_cast<int>(CensoringStyle::Black));
    m_censoringStyleCombo->addItem("Gaussian Blur", static_cast<int>(CensoringStyle::GaussianBlur));
    m_censoringStyleCombo->addItem("Pixelation", static_cast<int>(CensoringStyle::Pixelation));
    m_censoringStyleCombo->setCurrentIndex(0);

    censoringLayout->addWidget(styleLabel);
    censoringLayout->addWidget(m_censoringStyleCombo);

    m_intensityLabel = new QLabel("Blur Intensity: 20", this);
    m_intensitySlider = new QSlider(Qt::Horizontal, this);
    m_intensitySlider->setRange(1, 100);
    m_intensitySlider->setValue(20);
    m_intensitySlider->setVisible(false);
    m_intensityLabel->setVisible(false);

    censoringLayout->addWidget(m_intensityLabel);
    censoringLayout->addWidget(m_intensitySlider);

    layout->addWidget(censoringGroup);

    connect(m_censoringStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CensorTab::onCensoringStyleChanged);
    connect(m_intensitySlider, &QSlider::valueChanged, this, &CensorTab::updateIntensityLabel);
    connect(m_intensitySlider, &QSlider::valueChanged, this, [this](int) {emitCensoringConfig();});
    connect(m_censoringStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { emitCensoringConfig();});

    layout->addStretch();
}

void CensorTab::emitCensoringConfig() {
    CensoringConfig config;
    config.style = static_cast<CensoringStyle>(m_censoringStyleCombo->currentData().toInt());
    config.blurIntensity = m_intensitySlider->value();
    config.pixelSize = m_intensitySlider->value();
    emit censoringConfigChanged(config);
}

void CensorTab::broadcastCurrentSettings() {
    emitCensoringConfig();
}

void CensorTab::onCensoringStyleChanged(int index) {
    int style = m_censoringStyleCombo->itemData(index).toInt();
    
    if (style == static_cast<int>(CensoringStyle::Black)) {
        m_intensityLabel->setVisible(false);
        m_intensitySlider->setVisible(false);
    } else if (style == static_cast<int>(CensoringStyle::GaussianBlur)) {
        m_intensityLabel->setVisible(true);
        m_intensitySlider->setVisible(true);
        m_intensityLabel->setText("Blur Intensity: " + QString::number(m_intensitySlider->value()));
        m_intensitySlider->setRange(1, 100);
    } else if (style == static_cast<int>(CensoringStyle::Pixelation)) {
        m_intensityLabel->setVisible(true);
        m_intensitySlider->setVisible(true);
        m_intensityLabel->setText("Pixel Size: " + QString::number(m_intensitySlider->value()));
        m_intensitySlider->setRange(2, 50);
    }
}

void CensorTab::updateIntensityLabel(int value) {
    int style = m_censoringStyleCombo->currentData().toInt();
    
    if (style == static_cast<int>(CensoringStyle::GaussianBlur)) {
        m_intensityLabel->setText("Blur Intensity: " + QString::number(value));
    } else if (style == static_cast<int>(CensoringStyle::Pixelation)) {
        m_intensityLabel->setText("Pixel Size: " + QString::number(value));
    }
}