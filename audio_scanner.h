#ifndef AUDIO_SCANNER_H
#define AUDIO_SCANNER_H
#include <QObject>
#include <QThread>
#include <QDebug>
#include "add_functions.h"
#include "../Test_Audio_processing/audioin.h"

class audio_worker : public QObject
{
    Q_OBJECT
private:
    AudioIn *logDevice = NULL;
    PaStreamParameters params;
public:
    explicit audio_worker(QObject *parent = 0);
    ~audio_worker();
public slots:
    void init_audio(int seconds);
    void test_audio(void);
    void getCurValue(double data);
    void getNewAudioValue(void);
signals:
    void audio_test(void);
    void audio_init(int);
    void CurValue(double);
    void getNewValue(void);
};



class audio_scanner : public QObject
{
    Q_OBJECT
private:
    QThread workerThread;
public:
    explicit audio_scanner(QObject *parent = 0);
    ~audio_scanner();
public slots:
    void init_audio(int seconds);
    void test_audio(void);
    void getCurValue(double data);
    void getNewAudioValue(void);
signals:
    void audio_test(void);
    void audio_init(int);
    void CurValue(double);
    void getNewValue(void);
};

#endif // AUDIO_SCANNER_H
