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

    QGroupBox *bboxScaleGroup = new QGroupBox("Censoring box scale", this);
    QVBoxLayout *bboxScaleLayout = new QVBoxLayout(bboxScaleGroup);

    m_bboxScaleLabel = new QLabel("Scale: 1.00x", this);
    m_bboxScaleSlider = new QSlider(Qt::Horizontal, this);
    m_bboxScaleSlider->setRange(50, 200);
    m_bboxScaleSlider->setValue(100);
    m_bboxScaleSlider->setTickPosition(QSlider::TicksBelow);
    m_bboxScaleSlider->setTickInterval(10);

    bboxScaleLayout->addWidget(m_bboxScaleLabel);
    bboxScaleLayout->addWidget(m_bboxScaleSlider);

    layout->addWidget(bboxScaleGroup);

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
    connect(m_intensitySlider, &QSlider::valueChanged, this, [this](int) { emitCensoringConfig(); });
    connect(m_censoringStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { emitCensoringConfig(); });
    connect(m_bboxScaleSlider, &QSlider::valueChanged, this, &CensorTab::updateBboxScaleLabel);
    connect(m_bboxScaleSlider, &QSlider::valueChanged, this, [this](int) { emitCensoringConfig(); });

    layout->addStretch();
}

void CensorTab::emitCensoringConfig() {
    CensoringConfig config;
    config.style = static_cast<CensoringStyle>(m_censoringStyleCombo->currentData().toInt());
    config.blurIntensity = m_intensitySlider->value();
    config.pixelSize = m_intensitySlider->value();
    config.scaleFactor = m_bboxScaleSlider->value() / 100.0f;  // Convert 50-200 to 0.5-2.0
    emit censoringConfigChanged(config);
}

void CensorTab::updateBboxScaleLabel(int value) {
    float scale = value / 100.0f;
    m_bboxScaleLabel->setText(QString("Scale: %1x").arg(scale, 0, 'f', 2));
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