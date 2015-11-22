#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPortInfo>
#include <QVector>
#include <QPair>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_picker.h>
#include <qwt/qwt_plot_picker.h>
#include <qwt/qwt_picker_machine.h>
#include "serial_controller.h"
#include "add_functions.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    serial_controller *stepper = NULL, *monochromator = NULL;
signals:
    void executeCommandStepper(QString command, double delay);

    void executeCommandMono(QString command, double delay);

private:
    QString current_Stepper_Command;

    QString current_Mono_Command;

    double current_Stepper_Position;

    QPen pen;
    QwtPlotCurve Curve;
    QwtPlotGrid Grid;

    QVector<QPair<int, double> > dispValues;
    QMap<double, double> plotData;

    void replot();

    double getMaxValue();


private slots:
    void on_connectMono_clicked();

    void on_connect_stepper_clicked();

    void on_Send_Data_Mono_clicked();

    void on_Send_Data_Stepper_clicked();

    void on_stepper_command_currentIndexChanged(const QString &arg1);

    void on_mono_command_currentIndexChanged(const QString &arg1);

    void on_startScan_clicked();

public slots:
    void Received_Stepper_Data(QString &data);

    void Received_Mono_Data(QString &data);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
