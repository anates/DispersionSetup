#ifndef STEPPERM_H
#define STEPPERM_H

#include <QObject>
#include <QThread>
#include <QRegExp>
#include "serial_controller.h"
#include "add_functions.h"
#include "data_cleaner.h"

#define TIME_OUT 5000


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
    int timerID;

    void start_timer(void);
protected:
    void timerEvent(QTimerEvent *event);
public:
    explicit stepperMworker(serialPortInfos data, QObject *parent = 0);
    ~stepperMworker();
signals:
    void executeCommand(QString, int);
    void updatePosition(double);
    void updateMovementTime(double);
    void movementFinished(void);
    void connError(bool);
    //Internal
    void ReadingError(QString);
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
    void connectionError(bool error);

};


class stepperM : public QObject
{
    Q_OBJECT
private:
    QThread workerThread;

    double min_pos, max_pos;
    double cur_pos = -1;
public:
    explicit stepperM(QString port, double min, double max, QObject *parent = 0);
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
    void connErr(bool);
public slots:
    void getEstimatedMovementTime(double pos);
    bool moveAbs(double newPos);
    bool moveRel(double newPos);
    bool home(void);
    bool getCurPos(void);
    void updatePosition(double newPos);
    void updateTime(double Time);
    void StpMovFinished(void);
    void connectionError(bool error);
};

#endif // STEPPERM_H
