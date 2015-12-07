#include "stepperm.h"

stepperMworker::stepperMworker(serialPortInfos data, QObject *parent) : QObject(parent)
{
    this->stepper = new serial_controller(data.portName, 1000, 57600, 1, false, false, false, false);
    connect(this->stepper, &serial_controller::response, this, &stepperMworker::newData);
    connect(this, &stepperMworker::executeCommand, this->stepper, &serial_controller::transaction);
}

stepperMworker::~stepperMworker()
{

}

void stepperMworker::newData(QString data)
{
    this->response.clear();
    this->response = data;
    data.clear();
    if(this->movementTimeCheck)
    {
        if(this->response.contains("PT"))
        {
            QList<QString> response = this->response.split("PT");
            this->estimated_movement_time = (response.length() == 2)?response[1].toDouble():0;
        }
        this->movementTimeCheck = false;
    }
    else
    {
        if(this->response.contains("TP"))
        {
            QList<QString> response = this->response.split("TP");
            emit this->updatePosition((response.length() == 2)?response[1].toDouble():-1);
            this->curPos = (response.length() == 2)?response[1].toDouble():this->curPos;
        }
    }
}

void stepperMworker::moveAbsolute(double pos)
{
    emit this->executeCommand("1PA" + QString::number(pos), 0);
    this->curPos = pos;
    emit this->updatePosition(pos);
    this->estimated_movement_time = 0;
}

void stepperMworker::moveRelative(double pos)
{
    emit this->executeCommand("1PR" + QString::number(pos), 0);
    this->curPos += pos;
    emit this->updatePosition(this->curPos += pos);
    this->estimated_movement_time = 0;
}

void stepperMworker::home()
{
    emit this->executeCommand("1OR?",0);
}

void stepperMworker::getMovementTime(double newPos)
{
    this->movementTimeCheck = true;
    emit this->executeCommand("1PT"+QString::number(newPos), 0);
}

void stepperMworker::move(double pos, bool abs)
{
    this->getMovementTime((abs == true)?fabs(this->curPos - pos):fabs(pos));
    int i = 0;
    while(this->estimated_movement_time == 0 && i < 100)
    {
        QThread::msleep(10);
        i++;
    }
    if(abs)
        this->moveAbsolute(pos);
    else
        this->moveRelative(pos);
    QThread::sleep(this->estimated_movement_time);
    this->estimated_movement_time = 0;
}






stepperM::stepperM(QString port, QObject *parent) : QObject(parent)
{
    serialPortInfos data;
    data.portName = port;
    stepperMworker *newStepper = new stepperMworker(data);
    newStepper->moveToThread(&(this->workerThread));
    connect(&(this->workerThread), &QThread::finished, newStepper, &stepperMworker::deleteLater);
    connect(this, &stepperM::AbsMove, newStepper, &stepperMworker::moveAbsolute);
    connect(this, &stepperM::RelMove, newStepper, &stepperMworker::moveRelative);
    connect(this, &stepperM::homeMove, newStepper, &stepperMworker::home);
}

stepperM::~stepperM()
{

}

double stepperM::getCurPos()
{
    return this->cur_pos;
}

void stepperM::updatePosition(double newPos)
{
    if(newPos != -1)
        this->cur_pos = newPos;
}

bool stepperM::moveAbs(double newPos)
{
    if(newPos < this->min_pos || newPos > this->max_pos)
        return false;
    else
    {
        emit this->AbsMove(newPos);
        return true;
    }
}

bool stepperM::moveRel(double newPos)
{
    if((this->cur_pos + newPos) < this->min_pos || (this->cur_pos + newPos) > this->max_pos)
        return false;
    else
    {
        emit this->moveRel(newPos);
        return true;
    }
}

bool stepperM::home()
{
    emit this->homeMove();
    return true;
}
