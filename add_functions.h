#ifndef ADD_FUNCTIONS
#define ADD_FUNCTIONS
#include <QVector>
#include <QPair>
#include <QMap>
#include <QString>
#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QDateTime>

#define UNUSED(x) (void)(x)


void vectorToMap(const QVector<QPair<double, double> > &indata, QMap<double, double> &outdata);//Assumes that all values are already ordered

void debug_out(QString data, int debug_level = 0);


struct movementData{
    int moveDir = 0;
    double dist = 0;
    double waitingTime = 0;
    bool toMove = false;
};


class scanData{
    double start;
    double stop;
    int steps;
    double stepsize;
    bool cleared;
    int curStep;
public:
    scanData();

    void refill(double start, double stop, int steps);
    void clear(void);
};



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

