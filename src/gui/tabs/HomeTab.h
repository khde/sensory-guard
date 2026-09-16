#ifndef HOMETAB_H
#define HOMETAB_H

#include <QWidget>

class GuardEngine;
class QPushButton;

class HomeTab : public QWidget {
    Q_OBJECT

public:
    explicit HomeTab(GuardEngine *engine, QWidget *parent = nullptr);

private slots:
    void updateActiveState(bool active);

private:
    GuardEngine *m_engine;
    QPushButton *m_activateButton;
};

#endif