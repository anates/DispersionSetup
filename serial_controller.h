#ifndef SERIAL_CONTROLLER_H
#define SERIAL_CONTROLLER_H
#include <QObject>
#include <QThread>
#include <QVector>
#include <memory>
#include <QtSerialPort/QtSerialPort>
#include <stdlib.h>
#include <stdio.h>

class serial_controller_worker: public QObject
{
    Q_OBJECT
private:
    QString portName;
    QString request;
    QString storage;
    int delay_write;
    int numStopBits;
    bool useParity;
    bool parity;
    bool useHex;
    int waitTimeout;
    QMutex mutex;
    QWaitCondition cond;
    int baudrate;
    QSerialPort *serial;
    bool quit;
    bool sendNewData = false;
    bool recvLoop = false;

    void convertStringToHex(const QString &input, QByteArray &output);

public slots:
    void transaction(const QString &request, double delay);
    void read_data(void);
signals:
    void response(QString s);
    void error(const QString &s);
    void timeout(const QString &s);
public:
    serial_controller_worker(const QString &portname, int waitTimeout, int BaudRate, int numStopBits, bool parity, bool useParity, bool useHex);
    ~serial_controller_worker();

    void process_data(void);
};

class serial_controller: public QObject
{
    Q_OBJECT
private:
    QThread workerThread;
    QThread updateThread;
    QString portName;
    QString request;
    int waitTimeout;
    QMutex mutex;
    QWaitCondition cond;
    int baudrate;
    QSerialPort serial;
    bool Hex;
//    QString portName;
//    QString request;
//    int numStopBits;
//    bool useParity;
//    bool parity;
//    int waitTimeout;
//    QMutex mutex;
//    QWaitCondition cond;
//    int baudrate;
//    QSerialPort *serial;
//    bool quit;
//    bool sendNewData = false;
//    bool recvLoop = false;

public:
    serial_controller(const QString &portName, int waitTimeout, int BaudRate, int numStopBits, bool parity, bool useParity, bool Hex, bool useHex);
    ~serial_controller();

public slots:
    void transaction(const QString &request, double delay);
    void response_slot(QString response);
    void read_data(void);
signals:
    void newTransaction(const QString &request, double delay);
    void response(QString s);
    void error(const QString &s);
    void timeout(const QString &s);
    void CountValue(double val);
};

#endif // SERIAL_CONTROLLER_H
