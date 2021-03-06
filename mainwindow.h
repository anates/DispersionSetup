#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPortInfo>
#include <QVector>
#include <QThread>
#include <QPair>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_picker.h>
#include <qwt/qwt_plot_picker.h>
#include <qwt/qwt_picker_machine.h>
#include "serial_controller.h"
#include "add_functions.h"
#include "../Test_Audio_processing/audioin.h"
#include "monochrom.h"
#include "stepperm.h"

#define MAX 16
#define MIN 0

#define UNUSED(x) (void)(x)

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
    stepperM *stepp = NULL;
    monoChrom *mono = NULL;
signals:
    //void executeCommandStepper(QString command, double delay);

    //void executeCommandMono(QString command, double delay);
    //Monosignals
    bool moveToWL(int WL);
    bool resetMono(void);
    void getCurrentPos(void);
    //Steppersignals
    bool homeMirror();
    bool AbsStepper(double newPos);
    bool RelStepper(double newPos);
    void getMovementTime(double pos);
    //To audio
    void getNewValue(void);
    void getNewFFT(void);

    void maxValue(double val);

private:
    QString current_Stepper_Command;

    QString current_Mono_Command;

    QString mono_response;
    QString stepper_response;

    bool multiAqu = false;
    bool scanRun = false;

    int current_Mono_Position = 0;

    int current_Measurement;

    int stepper_min_limit = MIN;
    int stepper_max_limit = MAX;

    void write_unformatted_file(const QVector<QPair<double, double> > &Data/*const QMap<double, double> &Data*/, QString fileName);

    void ScanMovementStopped(void);

    double current_Stepper_Position;

    QPen pen;
    QwtPlotCurve Curve;
    QwtPlotGrid Grid;

    QVector<QPair<double, double> > dispValues;
    QVector<QPair<double, double> > cleanValues;
    QMap<double, double> plotData;

    AudioIn *logDevice = NULL;
    PaStreamParameters params;

    double movementTime = 0;

    double curAmplitude = 0;

    QString fileName = "";

    int num_steps = 100;
    double step_size = 0;
    int steps = 0;
    int curStep = 0;

    scanData ScanPosData;

    int wlSteps = 0;

    QVector<double> curScanVals;
    QVector<QPair<int, QPair<double, double> > > DispResults;

    void replot();

    void cleanPlot();

    QPair<double, double> getMaxValue(void);

    void init_scan();

    void write_unformatted_file(const QVector<QPair<int, QPair<double, double> > > &Data, QString fileName);

    void moveMonoToPosition(int pos);

    void moveStepperToAbsPosition(double pos);

    void moveStepperToRelPosition(double pos);

    void homeStepper(void);

    void getEstimatedMovementTime(double relPos);

    void doFullScan(void);

private slots:
    void on_connectMono_clicked();

    void on_connect_stepper_clicked();

    void on_Send_Data_Mono_clicked();

    void on_Send_Data_Stepper_clicked();

    void on_stepper_command_currentIndexChanged(const QString &arg1);

    void on_mono_command_currentIndexChanged(const QString &arg1);

    void on_startScan_clicked();

    void on_MovStopped_clicked();

    void on_FullScan_clicked();

    void on_ResetMono_clicked();

    void on_Mono_Value_1_textChanged(const QString &arg1);

    void on_GotoPositionButton_clicked();

public slots:
    void Received_Stepper_Data(QString &data);

    void Received_Mono_Data(QString &data);

    void getCurValue(double val);

    void getCurFFT(QVector<double> val);

    void CurPosUpdate(double pos);

    void movementTimeUpdate(double Time);

    void stepperConnectionError(bool error);

    void monoConnectionError(bool error);
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
