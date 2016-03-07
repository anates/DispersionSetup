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
    this->waveLenght = 0;
    this->start = 0;
    this->curStep = 0;
    this->stop = 0;
    this->steps = 0;
    this->cleared = false;
    this->stepsize = (this->steps==0)?1:(fabs(stop - start)/(double)steps);
}



void scanData::refill(double start, double stop, int steps, int wavelength)
{
    if(this->cleared == false)
        this->clear();
    this->waveLenght = wavelength;
    this->start = start;
    this->stop = stop;
    this->steps = steps;
    this->curStep = 0;
    this->cleared = false;
    this->stepsize = (steps==0)?1:(fabs(stop - start)/(double)steps);
}

void scanData::clear()
{
    this->start = 0;
    this->stop = 0;
    this->steps = 0;
    this->stepsize = 0;
    this->curStep = 0;
    this->waveLenght = 0;
    this->wavelenghtSteps = 0;
    this->cleared = true;
}
