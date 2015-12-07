#ifndef STEPPERM_H
#define STEPPERM_H

#include <QObject>
#include <QThread>
#include "serial_controller.h"
#include "add_functions.h"

class stepperMworker : public QObject
{
    Q_OBJECT
private:
    serial_controller *stepper = NULL;


    void getMovementTime(double newPos);


    QString response;
    bool movementTimeCheck = false;
    double estimated_movement_time = 0;
    double curPos = 0;
public:
    explicit stepperMworker(serialPortInfos data, QObject *parent = 0);
    ~stepperMworker();
signals:
    void executeCommand(QString, int);
    void updatePosition(double);
public slots:
    void newData(QString data);
    void moveAbsolute(double pos);
    void moveRelative(double pos);
    void home(void);
    void move(double pos, bool abs);

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

    double getCurPos(void);

signals:
    void AbsMove(double);
    void RelMove(double);
    void homeMove();
public slots:
    bool moveAbs(double newPos);
    bool moveRel(double newPos);
    bool home(void);
    void updatePosition(double newPos);
};

#endif // STEPPERM_H
