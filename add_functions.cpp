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
    qDebug() << data << " " << QDateTime::currentMSecsSinceEpoch();
}
