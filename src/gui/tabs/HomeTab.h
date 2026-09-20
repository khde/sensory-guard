#ifndef HOMETAB_H
#define HOMETAB_H

#include <QWidget>

class GuardEngine;
class QPushButton;
class QLabel;

class HomeTab : public QWidget {
    Q_OBJECT

public:
    explicit HomeTab(GuardEngine *engine, QWidget *parent = nullptr);

private slots:
    void updateActiveState(bool active);
    void updateFps(double currentFps, double targetFps);

private:
    GuardEngine *m_engine;
    QPushButton *m_activateButton;
    QLabel *m_fpsLabel;
};

#endif