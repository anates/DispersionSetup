#ifndef STEPPERM_H
#define STEPPERM_H

#include <QObject>
#include <QThread>
#include <QRegExp>
#include "serial_controller.h"
#include "add_functions.h"
#include "data_cleaner.h"


class stepperMworker : public QObject
{
    Q_OBJECT
private:
    serial_controller *stepper = NULL;

    QString response;
    movementData data;
    bool movementTimeCheck = false;
    double estimated_movement_time = 0;
    double curPos = 0;
    data_cleaner *cleaner = NULL;
public:
    explicit stepperMworker(serialPortInfos data, QObject *parent = 0);
    ~stepperMworker();
signals:
    void executeCommand(QString, int);
    void updatePosition(double);
    void updateMovementTime(double);
    void movementFinished(void);
public slots:
    void getMovementTime(double newPos);
    bool waitMovement(double distance);
    void newData(QString data);
    void moveAbsolute(double pos);
    void moveRelative(double pos);
    void home(void);
    bool getCurPos(void);
    void prepareMovement(movementData data);
    void move(void);

};


class stepperM : public QObject
{
    Q_OBJECT
private:
    QThread workerThread;

    double min_pos = 4, max_pos = 16;
    double cur_pos = 4;
public:
    explicit stepperM(QString port, QObject *parent = 0);
    ~stepperM();

    double getCurPosVal(void);

signals:
    void AbsMove(double);
    void RelMove(double);
    void homeMove();
    bool WhereAmI(void);
    void curPosUpdate(double);
    void updateEstTime(double);
    void estMovTime(double);
    void move(movementData data);
    void movementFinished(void);
public slots:
    void getEstimatedMovementTime(double pos);
    bool moveAbs(double newPos);
    bool moveRel(double newPos);
    bool home(void);
    bool getCurPos(void);
    void updatePosition(double newPos);
    void updateTime(double Time);
    void StpMovFinished(void);
};

#endif // STEPPERM_H
