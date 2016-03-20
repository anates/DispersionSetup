#ifndef BASICSCANNER_H
#define BASICSCANNER_H
#include <add_functions.h>

#include <QObject>

enum ScanState {Stopped, Movement, SingleScan, FullScan, FTIRScan};
enum devices {Mono, Stepper};


class BasicScanner : public QObject
{
    Q_OBJECT
    ScanState curState;
    scanData jobData;
    stepperInfo jobStepper;

    devices Devices;//What moved last?

private slots:
    void doStepperScan(void);
    void doMonoScan(void);
    void doFTIRScan(void);
    void processScanValues(void);
    void initDevices(void);

public:
    explicit BasicScanner(QObject *parent = 0);

signals:
    bool moveToAbsPosition(double pos);
    bool moveToRelPosition(double pos);
    bool getNewADCData(void);
    void doNextStep(void);
    void ScanPercentage(double);
    void ScanFinished(void);
    void startstepperScan(void);
    void startmonoScan(void);
    void startFTIRScan(void);
    void moveMonoToPos(int pos);
    void FinishedScanData(scanData);
    void init(void);
public slots:
    void doScan(scanData data);
    void moveToNextPosition(void);
    void movementStopped(void);
    void gotNewADCData(double data);
};

#endif // BASICSCANNER_H
