#ifndef CENSORTAB_H
#define CENSORTAB_H

#include "gui/overlay/CensoringConfig.h"

#include <QWidget>
#include <QComboBox>
#include <QSlider>
#include <QLabel>

class CensorTab : public QWidget {
    Q_OBJECT

public:
    explicit CensorTab(QWidget *parent = nullptr);
    void broadcastCurrentSettings();

private:
    void updateIntensityLabel(int value);
    void updateBboxScaleLabel(int value);
    void onCensoringStyleChanged(int index);
    void emitCensoringConfig();

    QComboBox *m_censoringStyleCombo;
    QSlider *m_intensitySlider;
    QLabel *m_intensityLabel;
    QSlider *m_bboxScaleSlider;
    QLabel *m_bboxScaleLabel;

signals:
    void censoringConfigChanged(const CensoringConfig& config);
};

#endif