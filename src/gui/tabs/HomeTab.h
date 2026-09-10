#ifndef HOMETAB_H
#define HOMETAB_H

#include <QWidget>

class IShieldController;
class QPushButton;

class HomeTab : public QWidget {
    Q_OBJECT

public:
    explicit HomeTab(IShieldController *controller, QWidget *parent = nullptr);

private slots:
    void updateActiveState(bool active);

private:
    IShieldController *m_controller;
    QPushButton *m_activateButton;
};

#endif