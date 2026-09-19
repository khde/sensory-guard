#ifndef CUSTOMIZETAB_H
#define CUSTOMIZETAB_H

#include "gui/overlay/CensoringConfig.h"

#include <QWidget>
#include <QComboBox>
#include <QSlider>
#include <QLabel>

class CustomizeTab : public QWidget {
    Q_OBJECT

public:
    explicit CustomizeTab(QWidget *parent = nullptr);
    void broadcastCurrentSettings();

private:
    void updateIntensityLabel(int value);
    void onCensoringStyleChanged(int index);
    void emitCensoringConfig();

    QComboBox *m_censoringStyleCombo;
    QSlider *m_intensitySlider;
    QLabel *m_intensityLabel;

signals:
    void censoringConfigChanged(const CensoringConfig& config);
};

#endif