#ifndef MONOCHROM_H
#define MONOCHROM_H

#include <QObject>
#include <QThread>
#include "serial_controller.h"
#include "add_functions.h"

class monoChromworker : public QObject
{
    Q_OBJECT
private:
    serial_controller *monochromator = NULL;
    int currentWL = 0;
public:
    explicit monoChromworker(serialPortInfos data, QObject *parent = 0);
    ~monoChromworker();
signals:
    void executeCommand(QString, int);
    void gotNewData(QString);
    void currentWLVal(int);
    void connErr(bool);
public slots:
    void newData(QString data);
    bool moveToWL(int WL);
    void Reset(void);
    int getCurrentWL(void);
    void connectionError(bool error);
};


class monoChrom : public QObject
{
    Q_OBJECT
    QThread workerThread;

public:
    explicit monoChrom(QString port, QObject *parent = 0);
    ~monoChrom();

signals:
    void currentWL(int);
    void getCurrentWL(void);
    bool moveToWL(int);
    void reset(void);
    void renewCurrentWL(int);
    void connectionError(bool);

public slots:
    void setCurrentWL(int data);
    bool resetMono(void);
    bool moveMonoToWL(int WL);
    void getCurrentMonoWL(void);
    void gotNewData(QString data);
    void connErr(bool error);
};

#endif // MONOCHROM_H
