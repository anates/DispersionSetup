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
#define MAX 16
#define MIN 0

enum ScanType {Single, Full, FTIR};

enum MovingPos {Border, FullRange, None}; //Moving to border, to full range, or nothing

enum MovingDir {Absolute, Relative, Homing};

void vectorToMap(const QVector<QPair<double, double> > &indata, QMap<double, double> &outdata);//Assumes that all values are already ordered

void debug_out(QString data, int debug_level = 0);

struct movementData{
    MovingDir moveDir = Homing;
    double dist = 0;
    double waitingTime = 0;
    bool toMove = false;
};

class scanData{
public:
    double start;
    double stop;
    double curPos;
    int steps;
    double stepsize;
    bool cleared;
    int curStep;
    int curWaveLenght;
    int startWaveLength;
    int stopWaveLength;
    int wavelenghtSteps;
    QVector<QPair<double, double> > values;
    ScanType type;

    scanData();
    scanData(const scanData &data);
    void refill(double start, double stop, int steps, int startwavelength, int stopwavelength, int wavelengthsteps);
    void clear(void);
    void operator=(const scanData &D )
    {
        this->start = D.start;
        this->stop = D.stop;
        this->curPos = D.curPos;
        this->steps = D.steps;
        this->stepsize = D.stepsize;
        this->cleared = D.cleared;
        this->curStep = D.curStep;
        this->curWaveLenght = D.curWaveLenght;
        this->startWaveLength = D.startWaveLength;
        this->stopWaveLength = D.stopWaveLength;
        this->wavelenghtSteps = D.wavelenghtSteps;
        this->values = D.values;
        this->type = D.type;
    }
};

class stepperInfo{
public:
    double curPos;
    double curSpeed;
    double estMovementTime;
    bool initStepper;
    bool connected;

    stepperInfo();

    void init(bool connected);
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

