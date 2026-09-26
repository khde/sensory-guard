#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QDialog>

class AboutWindow : public QDialog {
    Q_OBJECT

public:
    explicit AboutWindow(const QString &version, QWidget *parent = nullptr);

private:
    void setupUi();
};

#endif