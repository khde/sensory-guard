#ifndef SETTINGSTAB_H
#define SETTINGSTAB_H

#include <QWidget>
#include <QCheckBox>

class SettingsTab : public QWidget {
    Q_OBJECT

public:
    explicit SettingsTab(QWidget *parent = nullptr);
    bool minimizeToTrayEnabled() const;

private:
    QCheckBox *m_minimizeToTrayCheckBox;

signals:
    void minimizeToTrayChanged(bool enabled);
};

#endif