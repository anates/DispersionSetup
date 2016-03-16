#include "add_functions.h"

void vectorToMap(const QVector<QPair<double, double> > &indata, QMap<double, double> &outdata)//Assumes that all values are already ordered
{
    for(int i = 0; i < indata.size(); i++)
    {
        outdata[indata[i].first] = indata[i].second;
    }
}

void debug_out(QString data, int debug_level)
{
    QFile file("logfile.txt");
    file.open(QIODevice::Append);
    QTextStream out(&file);
    if(debug_level > 0)
        out << '\n' << data << " " << QDateTime::currentMSecsSinceEpoch();
    file.close();
    qDebug() << data;// << " " << QDateTime::currentMSecsSinceEpoch();
}

scanData::scanData()
{
    this->wavelenghtSteps = 0;
    this->curWaveLenght = 0;
    this->startWaveLength = 0;
    this->stopWaveLength = 0;
    this->start = 0;
    this->curStep = 0;
    this->curPos = 0;
    this->stop = 0;
    this->steps = 0;
    this->cleared = false;
    this->stepsize = (this->steps==0)?1:(fabs(stop - start)/(double)steps);
    this->values.clear();
}

scanData::scanData(const scanData & data)
{
    this->wavelenghtSteps = data.wavelenghtSteps;
    this->curWaveLenght = data.curWaveLenght;
    this->startWaveLength = data.startWaveLength;
    this->stopWaveLength = data.stopWaveLength;
    this->start = data.start;
    this->stop = data.stop;
    this->curPos = data.curPos;
    this->steps = data.steps;
    this->stepsize = data.stepsize;
    this->cleared = data.cleared;
    this->curStep = data.curStep;
    this->values = data.values;
    this->type = data.type;
}

void scanData::refill(double start, double stop, int steps, int startwavelength, int stopwavelength, int wavelengthsteps)
{
    if(this->cleared == false)
        this->clear();
    this->startWaveLength = startwavelength;
    this->stopWaveLength = stopwavelength;
    this->wavelenghtSteps = wavelengthsteps;
    this->curWaveLenght = this->startWaveLength;
    this->start = start;
    this->curPos = start;
    this->stop = stop;
    this->steps = steps;
    this->curStep = 0;
    this->cleared = false;
    this->stepsize = (steps==0)?1:(fabs(stop - start)/(double)steps);
    this->values.clear();
}

void scanData::clear()
{
    this->start = 0;
    this->stop = 0;
    this->steps = 0;
    this->stepsize = 0;
    this->curStep = 0;
    this->wavelenghtSteps = 0;
    this->curWaveLenght = 0;
    this->startWaveLength = 0;
    this->stopWaveLength = 0;
    this->cleared = true;
    this->curPos = 0;
    this->values.clear();
}

stepperInfo::stepperInfo()
{
    this->curPos = 0;
    this->estMovementTime = 0;
    this->curSpeed = 0;
    this->initStepper = true;
    this->connected = false;
}

void stepperInfo::init(bool connected)
{
    this->curPos = 0;
    this->estMovementTime = 0;
    this->curSpeed = 0;
    this->initStepper = true;
    this->connected = connected;

}
