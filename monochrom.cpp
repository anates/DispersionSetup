#include "monochrom.h"
//Worker
monoChromworker::monoChromworker(serialPortInfos data, QObject *parent) : QObject(parent)
{
    this->monochromator = new serial_controller(data.portName, 1000, 9600, 1, false, false, false, true);
    connect(this->monochromator, &serial_controller::response, this, &monoChromworker::newData);
    connect(this, &monoChromworker::executeCommand, this->monochromator, &serial_controller::transaction);
    connect(this->monochromator, &serial_controller::error_Val, this, &monoChromworker::connectionError);
}

monoChromworker::~monoChromworker()
{

}

void monoChromworker::newData(QString data)
{
    //Process first?
    emit this->gotNewData(data);
}

bool monoChromworker::moveToWL(int WL)
{
    //qDebug() << "Moving to WL: " << WL;
    debug_out("Moving to WL: " + QString::number(WL), 1);
    int value;
    if(WL == 0 || WL == this->currentWL)
        value = 0;
    else
        value = WL;
    QString wavelength;
    if(QString::number((qlonglong)value, 16).length() == 3)
        wavelength = "0" + QString::number((qlonglong)value, 16);
    else
        wavelength = QString::number((qlonglong)value, 16);
    emit this->executeCommand("10" + wavelength, 0);
    this->currentWL = WL;
    return true;

}

void monoChromworker::Reset()
{
    emit this->executeCommand("FFFFFF", 0);
    this->currentWL = 0;
}

int monoChromworker::getCurrentWL()
{
    emit this->currentWLVal(this->currentWL);
    return this->currentWL;
}

void monoChromworker::connectionError(bool error)
{
    emit this->connErr(error);
}

//Main class

monoChrom::monoChrom(QString port, QObject *parent) : QObject(parent)
{
    serialPortInfos data;
    data.portName = port;
    monoChromworker *newMonoChrom = new monoChromworker(data);
    newMonoChrom->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, newMonoChrom, &monoChromworker::deleteLater);
    connect(this, &monoChrom::moveToWL, newMonoChrom, &monoChromworker::moveToWL);
    connect(this, &monoChrom::reset, newMonoChrom, &monoChromworker::Reset);
    connect(this, &monoChrom::getCurrentWL, newMonoChrom, &monoChromworker::getCurrentWL);
    connect(newMonoChrom, &monoChromworker::gotNewData, this, &monoChrom::gotNewData);
    connect(newMonoChrom, &monoChromworker::currentWLVal, this, &monoChrom::setCurrentWL);
    connect(newMonoChrom, &monoChromworker::connErr, this, &monoChrom::connErr);
    workerThread.start();
}

monoChrom::~monoChrom()
{
    this->workerThread.quit();
    this->workerThread.wait();
}

bool monoChrom::resetMono()
{
    emit this->reset();
    return true;
}

void monoChrom::gotNewData(QString data)
{
    //qDebug() << "Data in mono is: " << data;
    debug_out("Data in Mono is: " + data, 1);
}

bool monoChrom::moveMonoToWL(int WL)
{
    bool res = emit this->moveToWL(WL);
    return res;
}

void monoChrom::getCurrentMonoWL()
{
    emit this->getCurrentWL();
}

void monoChrom::setCurrentWL(int data)
{
    emit this->renewCurrentWL(data);
}

void monoChrom::connErr(bool error)
{
    emit this->connectionError(error);
}
