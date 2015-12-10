#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "audioin.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_LogThis_clicked();

    void newMaxValue(double val);

private:
    Ui::MainWindow *ui;
    AudioIn *logDevice = NULL;
    PaStreamParameters params;
};

#endif // MAINWINDOW_H
