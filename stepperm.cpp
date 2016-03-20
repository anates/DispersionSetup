#include "stepperm.h"

stepperMworker::stepperMworker(serialPortInfos data, QObject *parent) : QObject(parent)
{
    this->stepper = new serial_controller(data.portName, 1000, 57600, 1, false, false, false, false);
    this->cleaner = new data_cleaner();
    //connect(this->stepper, &serial_controller::response, this->cleaner, &data_cleaner::newData);
    connect(this, &stepperMworker::executeCommand, this->stepper, &serial_controller::transaction);
    connect(this->stepper, &serial_controller::response, this, &stepperMworker::newData);
    connect(this, &stepperMworker::ReadingError, this, &stepperMworker::newData);
    connect(this->stepper, &serial_controller::error_Val, this, &stepperMworker::connectionError);
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
    bool writeStatus = emit this->executeCommand("1PT"+QString::number(distance), 0);
    if(writeStatus == false)
        return false;
    else
    {
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

void stepperMworker::connectionError(bool error)
{
    emit this->connError(error);
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
    double dist = (this->data.moveDir == Homing)?this->curPos:(this->data.moveDir == Absolute)?(fabs(this->curPos - this->data.dist)):this->data.dist;
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
    if(this->data.moveDir == Absolute)
        this->moveAbsolute(this->data.dist);
    else if(this->data.moveDir == Relative)
        this->moveRelative(this->data.dist);
    else
        this->home();
    QThread::sleep(1.1*(this->data.waitingTime));
    //qDebug() << "Finished sleeping the time " << this->data.waitingTime << "s!";
    debug_out("Finished sleeping the time " + QString::number(this->data.waitingTime) + "s!", 1);
    this->estimated_movement_time = 0;
    this->data.moveDir = Absolute;
    this->data.dist = 0;
    this->data.toMove = false;
    this->data.waitingTime = 0;
    emit this->movementFinished();
}

void stepperMworker::timerEvent(QTimerEvent *event)
{
    UNUSED(event);
    if(this->timerID != 0)
    killTimer(this->timerID);
    //qDebug() << "Timerevent is: " << event;
    //debug_out("Timerevent is: " + QString(event));
    this->timerID = 0;
    emit this->ReadingError(QString::number(-1));
}




stepperM::stepperM(QString port, double min, double max, QObject *parent) : QObject(parent)
{
    serialPortInfos data;
    data.portName = port;
    this->min_pos = min;
    this->max_pos = max;
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
    //connect(newStepper, &stepperMworker::updatePosition, this, &stepperM::updatePosition);//Maybe not needed
    connect(newStepper, &stepperMworker::movementFinished, this, &stepperM::StpMovFinished);
    connect(newStepper, &stepperMworker::connError, this, &stepperM::connectionError);
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

void stepperM::connectionError(bool error)
{
    emit this->connErr(error);
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
        debug_out("New Position is: " + QString::number(newPos), 1);
        emit this->curPosUpdate(newPos);
    }
    else
        debug_out("No new position!", 1);
}

MovingPos stepperM::moveAbs(double newPos)
{
    if(this->cur_pos == -1)
        this->cur_pos = 0;
    if(newPos < MIN)
    {
        debug_out("Moving abs: " + QString::number(newPos) + "!");
        movementData data;
        data.toMove = true;
        data.dist = MIN;
        data.moveDir = Absolute;
        data.waitingTime = 0;
        this->updatePosition(MIN);
        emit this->move(data);
        return Border;
    }
    else if(newPos > MAX)
    {
        debug_out("Moving abs: " + QString::number(newPos) + "!");
        movementData data;
        data.toMove = true;
        data.dist = MAX;
        data.moveDir = Absolute;
        data.waitingTime = 0;
        this->updatePosition(MAX);
        emit this->move(data);
        return Border;
    }
    else
    {
        debug_out("Moving abs: " + QString::number(newPos) + "!");
        movementData data;
        data.toMove = true;
        data.dist = newPos;
        data.moveDir = Absolute;
        data.waitingTime = 0;
        emit this->move(data);
        this->updatePosition(newPos);
        return FullRange;
    }
}

MovingPos stepperM::moveRel(double newPos)
{
    if(this->cur_pos == -1)
        this->cur_pos = 0;
    if(this->cur_pos + newPos < MIN)
    {
        movementData data;
        data.toMove = true;
        data.dist = MIN - this->cur_pos;
        data.moveDir = Relative;
        data.waitingTime = 0;
        emit this->move(data);
        this->updatePosition(MIN);
        return Border;
    }
    else if(this->cur_pos + newPos > MAX)
    {
        movementData data;
        data.toMove = true;
        data.dist = MAX - this->cur_pos;
        data.moveDir = Relative;
        data.waitingTime = 0;
        emit this->move(data);
        this->updatePosition(MAX);
        return Border;
    }
    else
    {
        movementData data;
        data.toMove = true;
        data.dist = newPos;
        data.moveDir = Relative;
        data.waitingTime = 0;
        emit this->move(data);
        this->updatePosition(this->cur_pos + newPos);
        return FullRange;
    }
}

void stepperM::getEstimatedMovementTime(double pos)
{
    emit this->estMovTime(pos);
}

bool stepperM::home()
{
    debug_out("In stepperM: Home", 1);
    movementData data;
    data.toMove = true;
    data.dist = 0;
    data.moveDir = Homing;
    data.waitingTime = 0;
    emit this->move(data);
    debug_out("Homing now!", 1);
    this->updatePosition(0);
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
