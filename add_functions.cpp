#include "add_functions.h"

void vectorToMap(const QVector<QPair<double, double> > &indata, QMap<double, double> &outdata)//Assumes that all values are already ordered
{
    for(int i = 0; i < indata.size(); i++)
    {
        outdata[indata[i].first] = indata[i].second;
    }
}
