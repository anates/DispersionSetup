#ifndef ADD_FUNCTIONS
#define ADD_FUNCTIONS
#include <QVector>
#include <QPair>
#include <QMap>
#include <QString>

void vectorToMap(const QVector<QPair<double, double> > &indata, QMap<double, double> &outdata);//Assumes that all values are already ordered

struct serialPortInfos{
    QString portName;
    int waitTimeout;
    int BaudRate;
    int numStopBits;
    bool parity;
    bool useParity;
    bool Hex;
    bool useHex;
};

#endif // ADD_FUNCTIONS

