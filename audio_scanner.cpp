#include "audio_scanner.h"

//Thread class

audio_worker::audio_worker(QObject *parent)
{
    this->logDevice = new AudioIn();
    //qDebug() << QString::number(Pa_Initialize());
    debug_out(QString::number(Pa_Initialize()));
    this->params.device = Pa_GetDefaultInputDevice();
    this->params.channelCount = 1;
    this->params.sampleFormat = PA_SAMPLE_TYPE;
    this->params.suggestedLatency = Pa_GetDeviceInfo( this->params.device )->defaultLowInputLatency;
    this->params.hostApiSpecificStreamInfo = NULL;
    connect(this->logDevice, &AudioIn::currentAmp, this, &audio_worker::getCurValue);
    connect(this, &audio_worker::getNewValue, this->logDevice, &AudioIn::maxAmplitude);
}



audio_worker::~audio_worker()
{
    if(this->logDevice != NULL)
        delete this->logDevice;
}

void audio_worker::getCurValue(double data)
{
    emit this->CurValue(data);
}

void audio_worker::getNewAudioValue()
{
    emit this->getNewValue();
}

void audio_worker::init_audio(int seconds)
{
    this->logDevice->initAudioIn(this->params, seconds);
    emit this->audio_init(seconds);
}


void audio_worker::test_audio()
{
    this->logDevice->testAudioIn();
    emit this->audio_test();
}

//Main class
audio_scanner::audio_scanner(QObject *parent)
{
    audio_worker *audio = new audio_worker();
    audio->moveToThread(&workerThread);

    connect(this, &audio_scanner::getNewValue, audio, &audio_worker::getNewAudioValue);
    connect(this, &audio_scanner::audio_init, audio, &audio_worker::init_audio);
    connect(this, &audio_scanner::audio_test, audio, &audio_worker::test_audio);

    connect(audio, &audio_worker::CurValue, this, &audio_scanner::getCurValue);
    connect(&workerThread, &QThread::finished, audio, &audio_worker::deleteLater);
    this->workerThread.start();
}

audio_scanner::~audio_scanner()
{
    this->workerThread.quit();
    this->workerThread.wait();
}

void audio_scanner::getCurValue(double data)
{
    emit this->CurValue(data);
}

void audio_scanner::getNewAudioValue()
{
    emit this->getNewValue();
}

void audio_scanner::init_audio(int seconds)
{
    emit this->audio_init(seconds);
}


void audio_scanner::test_audio()
{
    emit this->audio_test();
}
