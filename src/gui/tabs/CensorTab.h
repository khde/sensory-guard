#ifndef CENSORTAB_H
#define CENSORTAB_H

#include "config/UserSettings.h"

#include <QWidget>
#include <QComboBox>
#include <QSlider>
#include <QLabel>

class CensorTab : public QWidget {
    Q_OBJECT

public:
    explicit CensorTab(QWidget *parent = nullptr);
    void setSettings(const UserSettings &settings);

private:
    void updateIntensityLabel(int value);
    void updateBboxScaleLabel(int value);
    void onCensoringStyleChanged(int index);
    void emitSettingsChanged();

    QComboBox *m_censoringStyleCombo;
    QSlider *m_intensitySlider;
    QLabel *m_intensityLabel;
    QSlider *m_bboxScaleSlider;
    QLabel *m_bboxScaleLabel;

signals:
    void settingsChanged(int style, int intensity, float scale);
};

#endif