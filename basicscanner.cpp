#include "basicscanner.h"

BasicScanner::BasicScanner(QObject *parent) : QObject(parent)
{
    connect(this, &BasicScanner::doNextStep, this, &BasicScanner::processScanValues);
    connect(this, &BasicScanner::startstepperScan, this, &BasicScanner::doStepperScan);
    connect(this, &BasicScanner::startmonoScan, this, &BasicScanner::doMonoScan);
    connect(this, &BasicScanner::startFTIRScan, this, &BasicScanner::doFTIRScan);
    this->curState = Stopped;
}

void BasicScanner::doScan(scanData data)
{
    this->jobData = data;
    this->curState = (data.type == FTIR)?FTIRScan:FullScan;//((data.type == Single)?SingleScan:FullScan);

}

void BasicScanner::doStepperScan()
{
    if(this->jobData.curPos + this->jobData.stepsize <= this->jobData.stop)
    {
        emit this->moveToRelPosition(this->jobData.stepsize);
        this->jobStepper.curPos += this->jobData.stepsize;
    }
    else
        emit this->doNextStep();
}

void BasicScanner::processScanValues()
{
    if(this->jobData.curPos == this->jobData.stop && this->jobData.stopWaveLength == this->jobData.curWaveLenght)
    {
        emit this->ScanFinished();
        emit this->FinishedScanData(this->jobData);
        this->jobData.clear();
        this->Devices = Stepper;
    }
    else if(Devices == Stepper)
    {
        if(this->jobData.curPos < this->jobData.stop)
            emit this->startstepperScan();
        else if(this->jobData.curWaveLenght == 1)
            return;
    }
}

void BasicScanner::doFTIRScan()
{

}

void BasicScanner::doMonoScan()
{
    if(this->jobData.curWaveLenght + this->jobData.wavelenghtSteps <= this->jobData.stopWaveLength)
    {
        this->jobData.curWaveLenght += this->jobData.wavelenghtSteps;
        emit this->moveMonoToPos(this->jobData.curWaveLenght);
        //Delay?
    }
    else
    {
        this->jobData.curWaveLenght = this->jobData.stopWaveLength;
        emit this->moveMonoToPos(this->jobData.curWaveLenght);
    };
    this->Devices = Mono;
}

void BasicScanner::movementStopped()
{
    if(this->curState != Movement)
        emit this->getNewADCData();
    else
        this->curState = Stopped;
}

void BasicScanner::moveToNextPosition(void)
{
    if(this->curState == SingleScan || this->curState == FullScan)
    {
        this->jobData.curStep++;
        this->jobData.curPos += this->jobData.stepsize;
        emit this->moveToRelPosition(this->jobData.stepsize);
    }
    else if(this->curState == Movement)
    {
        emit this->moveToAbsPosition(MIN);
    }
    else if(this->curState == FTIRScan)
    {
        qDebug() << "Not implemented yet!";
    }
    else if(this->curState == Stopped)
    {
        return;
    }
}

void BasicScanner::gotNewADCData(double data)
{
    this->jobData.values.push_back(qMakePair(this->jobData.curPos, data));
    emit this->doNextStep();
}
