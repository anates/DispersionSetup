#include "stepperm.h"

stepperMworker::stepperMworker(serialPortInfos data, QObject *parent) : QObject(parent)
{
    this->stepper = new serial_controller(data.portName, 1000, 57600, 1, false, false, false, false);
    this->cleaner = new data_cleaner();
    //connect(this->stepper, &serial_controller::response, this->cleaner, &data_cleaner::newData);
    connect(this, &stepperMworker::executeCommand, this->stepper, &serial_controller::transaction);
    connect(this->stepper, &serial_controller::response, this, &stepperMworker::newData);
    connect(this, &stepperMworker::ReadingError, this, &stepperMworker::newData);
    //connect(this->cleaner, &data_cleaner::resultData, this, &stepperMworker::newData);
}

stepperMworker::~stepperMworker()
{

}

void stepperMworker::newData(QString data)
{
    this->response.clear();
    this->response = data;
    //qDebug() << "Timer-ID is: " << this->timerID;
    debug_out("Timer-ID is: " + QString::number(this->timerID));
    if(this->timerID != 0)
        killTimer(this->timerID);
    //qDebug() << "Timerevent is: " << event;
    //debug_out("Timerevent is: " + QString(event));
    this->timerID = 0;
    QString last = QString("\x00D\x00A");
    if(this->response == QString::number(-1))
    {
        //Due to comm bugs with the device, only as preliminary fix!
        this->estimated_movement_time = 5;
        emit this->updateMovementTime(this->estimated_movement_time);
        this->data.waitingTime = this->estimated_movement_time;
        //qDebug() << "New response time is: " << this->estimated_movement_time;
        debug_out("New response time is: " + QString::number(this->estimated_movement_time), 1);
        this->movementTimeCheck = false;
        if(this->data.toMove == true)
            this->move();
        return;
    }
    if(this->response.right(2) != last)
    {
        //qDebug() << "Response is: " << this->response;
        debug_out("Response is: " + this->response, 1);
        return;
    }
    //qDebug() << "Response is: " << this->response;
    debug_out("Response is: " + this->response, 1);
    data.clear();
    if(this->response.contains("PT"))
    {
        QList<QString> response = this->response.split("PT");
        this->estimated_movement_time = (response.length() == 2)?response[1].toDouble():0;
        emit this->updateMovementTime(this->estimated_movement_time);
        this->data.waitingTime = this->estimated_movement_time;
        //qDebug() << "New response time is: " << this->estimated_movement_time;
        debug_out("New response time is: " + QString::number(this->estimated_movement_time), 1);
        this->movementTimeCheck = false;
        if(this->data.toMove == true)
            this->move();
    }
    else if(this->response.contains("TP"))
    {
        QList<QString> response = this->response.split("TP");
        //qDebug() << "New Position is: " << response;
        QString tmp;
        for(int i = 0; i < response.size(); i++)
            tmp += response[i];
        debug_out("New Position is: " + tmp, 1);
        emit this->updatePosition((response.length() == 2)?response[1].toDouble():-1);
        this->curPos = (response.length() == 2)?response[1].toDouble():this->curPos;
    }
    else
    {
        //Due to comm bugs with the device, only as preliminary fix!
        this->estimated_movement_time = 5;
        emit this->updateMovementTime(this->estimated_movement_time);
        this->data.waitingTime = this->estimated_movement_time;
        //qDebug() << "New response time is: " << this->estimated_movement_time;
        debug_out("New response time is: " + QString::number(this->estimated_movement_time), 1);
        this->movementTimeCheck = false;
        if(this->data.toMove == true)
            this->move();
    }
}

bool stepperMworker::waitMovement(double distance)
{
    this->movementTimeCheck = true;
    emit this->executeCommand("1PT"+QString::number(distance), 0);
    this->start_timer();
    int i = 0;
    //qDebug() << "Waiting for suitable answer of moving time!";
    debug_out("Waiting for suitable answer of moving time!", 1);
    while(i < 1000 && this->movementTimeCheck == true)
    {
        i++;
        QThread::msleep(5);
    }
    if(this->movementTimeCheck == true)
        return false;
    else
        return true;
}

void stepperMworker::moveAbsolute(double pos)
{
    //double sleepTime = 0;
//    if(this->waitMovement(fabs(this->curPos - pos)))
//    {
//        qDebug() << "Waiting for movement!";
//        sleepTime = this->estimated_movement_time;
//    }
//    else
//    {
//        qDebug() << "Waiting two seconds!";
//        sleepTime = 2;
//    }
    emit this->executeCommand("1PA" + QString::number(pos), 0);
    this->start_timer();
//    QThread::sleep(sleepTime);
    this->curPos = pos;
    emit this->updatePosition(pos);
    this->estimated_movement_time = 0;
}

void stepperMworker::moveRelative(double pos)
{
    //double sleepTime = 0;
//    if(this->waitMovement(fabs(pos)))
//        sleepTime = this->estimated_movement_time;
//    else
//        sleepTime = 2;
    emit this->executeCommand("1PR" + QString::number(pos), 0);
    this->start_timer();
//    QThread::sleep(sleepTime);
//    this->curPos += pos;
    emit this->updatePosition(this->curPos += pos);
//    this->estimated_movement_time = 0;
}

bool stepperMworker::getCurPos()
{
    emit this->executeCommand("1TP?", 0);
    this->start_timer();
    return true;
}


void stepperMworker::home()
{
    //qDebug() << "Home now!";
    debug_out("Home now!", 1);
    emit this->executeCommand("1OR?",0);
    this->start_timer();
    QThread::sleep(5);
}

void stepperMworker::getMovementTime(double newPos)
{
    this->movementTimeCheck = true;
    //qDebug() << "Looking for movementTime!";
    debug_out("Looking for movementTime!", 1);
    emit this->executeCommand("1PT"+QString::number(newPos), 0);
    this->start_timer();
}

void stepperMworker::prepareMovement(movementData data)
{
    this->data = data;
    this->data.toMove = true;
    double dist = (this->data.moveDir == 1)?this->curPos:(this->data.moveDir == 2)?(fabs(this->curPos - this->data.dist)):this->data.dist;
    if(dist == 0)
        this->newData("1PT0\x00D\x00A");
    else
        this->getMovementTime(dist);
}

void stepperMworker::start_timer()
{
    this->timerID = startTimer(TIME_OUT);
}

void stepperMworker::move(void)
{
    if(this->data.moveDir == 2)
        this->moveAbsolute(this->data.dist);
    else if(this->data.moveDir == 3)
        this->moveRelative(this->data.dist);
    else
        this->home();
    QThread::sleep(1.1*(this->data.waitingTime));
    //qDebug() << "Finished sleeping the time " << this->data.waitingTime << "s!";
    debug_out("Finished sleeping the time " + QString::number(this->data.waitingTime) + "s!", 1);
    this->estimated_movement_time = 0;
    this->data.moveDir = 2;
    this->data.dist = 0;
    this->data.toMove = false;
    this->data.waitingTime = 0;
    emit this->movementFinished();
}

void stepperMworker::timerEvent(QTimerEvent *event)
{
    if(this->timerID != 0)
    killTimer(this->timerID);
    //qDebug() << "Timerevent is: " << event;
    //debug_out("Timerevent is: " + QString(event));
    this->timerID = 0;
    emit this->ReadingError(QString::number(-1));
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
    connect(this, &stepperM::estMovTime, newStepper, &stepperMworker::getMovementTime);
    connect(this, &stepperM::WhereAmI, newStepper, &stepperMworker::getCurPos);
    connect(this, &stepperM::move, newStepper, &stepperMworker::prepareMovement);
    connect(newStepper, &stepperMworker::updateMovementTime, this, &stepperM::updateTime);
    connect(newStepper, &stepperMworker::updatePosition, this, &stepperM::updatePosition);
    connect(newStepper, &stepperMworker::movementFinished, this, &stepperM::StpMovFinished);
    workerThread.start();
}

stepperM::~stepperM()
{
    this->workerThread.quit();
    this->workerThread.wait();
}


bool stepperM::getCurPos()
{
    bool a = emit this->WhereAmI();
    return a;
}

double stepperM::getCurPosVal()
{
    return this->cur_pos;
}

void stepperM::updatePosition(double newPos)
{
    if(newPos != -1)
    {
        this->cur_pos = newPos;
        //qDebug() << "New Position is: " << newPos;
        debug_out("New Position is: " + QString::number(newPos), 1);
        emit this->curPosUpdate(newPos);
    }
    else
        //qDebug() << "No new position!";
        debug_out("No new position!", 1);
}

bool stepperM::moveAbs(double newPos)
{
    if(newPos < this->min_pos || newPos > this->max_pos)
        return false;
    else
    {
        //emit this->AbsMove(newPos);
        //qDebug() << "Moving abs!";
        debug_out("Moving abs: " + QString::number(newPos) + "!");
        movementData data;
        data.toMove = true;
        data.dist = newPos;
        data.moveDir = 2;
        data.waitingTime = 0;
        emit this->move(data);
        return true;
    }
}

bool stepperM::moveRel(double newPos)
{
    if((this->cur_pos + newPos) < this->min_pos || (this->cur_pos + newPos) > this->max_pos)
        return false;
    else
    {
        //emit this->moveRel(newPos);
        movementData data;
        data.toMove = true;
        data.dist = newPos;
        data.moveDir = 3;
        data.waitingTime = 0;
        emit this->move(data);
        return true;
    }
}

void stepperM::getEstimatedMovementTime(double pos)
{
    emit this->estMovTime(pos);
}

bool stepperM::home()
{
    //qDebug() << "In stepperM: Home";
    debug_out("In stepperM: Home", 1);
    //emit this->homeMove();
    movementData data;
    data.toMove = true;
    data.dist = 0;
    data.moveDir = 1;
    data.waitingTime = 0;
    emit this->move(data);
    //qDebug() << "Homing now!";
    debug_out("Homing now!", 1);
//    data.toMove = true;
//    data.dist = this->min_pos;
//    data.moveDir = 2;
//    emit this->move(data);
    return true;
}

void stepperM::updateTime(double Time)
{
    //qDebug() << "Est. mov time is: " << Time;
    debug_out("Est. mov time is: " + QString::number(Time) + "s!", 1);
    emit this->updateEstTime(Time);
}

void stepperM::StpMovFinished()
{
    emit this->movementFinished();
}
