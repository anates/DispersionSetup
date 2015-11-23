#include "serial_controller.h"

serial_controller_worker::serial_controller_worker(const QString &portname, int waitTimeout, int BaudRate, int numStopBits, bool parity, bool useParity, bool useHex)
{
    this->portName = portname;
    this->waitTimeout = waitTimeout;
    this->baudrate = BaudRate;
    this->numStopBits = numStopBits;
    this->useParity = useParity;
    this->parity = parity;
    this->useHex = useHex;
    this->serial = new QSerialPort(this);
    this->storage = "";
    this->delay_write = 0;
    connect(this->serial, &QSerialPort::readyRead, this, &serial_controller_worker::read_data);
    this->serial->setPortName(this->portName);
    this->serial->setDataBits(QSerialPort::Data8);
    this->serial->setBaudRate(this->baudrate);
    this->serial->setStopBits((this->numStopBits==2)?QSerialPort::TwoStop:QSerialPort::OneStop);
    this->serial->setParity((this->useParity?(this->parity?QSerialPort::OddParity:QSerialPort::EvenParity):QSerialPort::NoParity));
    qDebug() << "Using " << this->baudrate << " and " << this->useParity << " as parity and " << this->numStopBits << " as stopbits and " << (this->useParity?(this->parity?QSerialPort::OddParity:QSerialPort::EvenParity):QSerialPort::NoParity) << " as parity";
    if (!serial->open(QIODevice::ReadWrite))
    {
        emit error(tr("Can't open %1, error code %2").arg(portName).arg(serial->error()));
        qDebug() << tr("Can't open %1, error code %2").arg(portName).arg(serial->error());
        return;
    }
    else
    {
        emit error(tr("Opened %1").arg(portName));
        qDebug() << tr("Opened %1").arg(portName);
    }
}

serial_controller_worker::~serial_controller_worker()
{
    if(this->serial->isOpen())
        this->serial->close();
    if(this->serial != NULL)
        delete this->serial;
}

void serial_controller_worker::read_data()
{
    QByteArray data = this->serial->readAll();
    storage += QString::fromLatin1(data.data());
    QString tmp = QString::fromLatin1(data.data());
    int val = (int)tmp.toStdString().c_str()[0];
    emit this->response(storage);
    if(val == 24)
        storage.clear();

}

void serial_controller_worker::convertStringToHex(const QString &input, QByteArray &output)
{
    QString d1,d2,d3;
    qDebug() << "From conversion: " + input;
    d1.append(input[0]);
    d1.append(input[1]);
    d2.append((input.length() > 2)?input[2]:QChar('0'));
    d2.append((input.length() > 3)?input[3]:QChar('0'));
    d3.append((input.length() > 4)?input[4]:QChar('0'));
    d3.append((input.length() > 5)?input[5]:QChar('0'));

    bool bOK;
    output.append((char) d1.toInt(&bOK,16));
    if(input.length() > 2)
        output.append((char) d2.toInt(&bOK,16));
    if(input.length() > 4)
        output.append((char) d3.toInt(&bOK,16));
}

void serial_controller_worker::process_data()
{
}

void serial_controller_worker::transaction(const QString &request, double delay)
{
//    qDebug() << "TransAction started!";
    this->delay_write = delay;
    QByteArray requestData;
    if(this->useHex == false)
    {
        QString request_enter = request + QString("\x00D\x00A");
        qDebug() << "Converted QString: " << request.toInt();
        QByteArray requestData = request_enter.toLocal8Bit();
    }
    else
    {
        this->convertStringToHex(request, requestData);
    }
    QThread::msleep((this->delay_write)*1000);
    qDebug() << "Writing data: " << requestData;
    int status = serial->write(requestData);
    qDebug() << "Writestatus is: " + QString::number(status);
}


//Serial_controller functions
serial_controller::serial_controller(const QString &portName, int waitTimeout, int BaudRate, int numStopBits, bool parity, bool useParity, bool Hex, bool useHex)
{
    serial_controller_worker *newWorker = new serial_controller_worker(portName, waitTimeout, BaudRate, numStopBits, parity, useParity, useHex);
    newWorker->moveToThread(&workerThread);
    this->Hex = Hex;
    connect(&workerThread, &QThread::finished, newWorker, &QObject::deleteLater);
    connect(this, &serial_controller::newTransaction, newWorker, &serial_controller_worker::transaction);
    connect(newWorker, &serial_controller_worker::response, this, &serial_controller::response_slot);
    workerThread.start();

//    this->portName = portName;
//    this->waitTimeout = waitTimeout;
//    this->baudrate = BaudRate;
//    this->numStopBits = numStopBits;
//    this->useParity = useParity;
//    this->parity = parity;
//    this->serial = new QSerialPort(this);
//    qDebug() << "Connect working: " << connect(this->serial, &QSerialPort::readyRead, this, &serial_controller::read_data);
//    this->serial->setPortName(this->portName);
//    this->serial->setDataBits(QSerialPort::Data8);
//    this->serial->setBaudRate(this->baudrate);
//    this->serial->setStopBits((this->numStopBits==2)?QSerialPort::TwoStop:QSerialPort::OneStop);
//    this->serial->setParity((this->useParity?(this->parity?QSerialPort::OddParity:QSerialPort::EvenParity):QSerialPort::NoParity));
//    qDebug() << "Using " << this->baudrate << " and " << this->useParity << " as parity and " << this->numStopBits << " as stopbits and " << (this->useParity?(this->parity?QSerialPort::OddParity:QSerialPort::EvenParity):QSerialPort::NoParity) << " as parity";
//    if (!serial->open(QIODevice::ReadWrite))
//    {
//        emit error(tr("Can't open %1, error code %2").arg(portName).arg(serial->error()));
//        qDebug() << tr("Can't open %1, error code %2").arg(portName).arg(serial->error());
//        return;
//    }
//    else
//    {
//        emit error(tr("Opened %1").arg(portName));
//        qDebug() << tr("Opened %1").arg(portName);
//    }
}

serial_controller::~serial_controller()
{
    workerThread.quit();
    workerThread.wait();
//    if(this->serial->isOpen())
//        this->serial->close();
//    if(this->serial != NULL)
//        delete this->serial;
}

void serial_controller::transaction(const QString &request, double delay)
{
    qDebug() << "Sent new transaction request " << request << " to worker!";
    emit this->newTransaction(request, delay);
//    QByteArray requestData = request.toLocal8Bit();
//    qDebug() << "Writing data: " << requestData;
//    serial->write(requestData);
//    qDebug() << "Data written";
}

void serial_controller::read_data()
{
//    qDebug() << "Reading data";
//    QByteArray data = this->serial->readAll();
//    qDebug() << "Data is: " << data;
//    emit this->response(QString::fromLatin1(data.data()));
}

void serial_controller::response_slot(QString response)
{
    emit this->response(response);
}

