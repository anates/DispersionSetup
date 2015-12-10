#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->logDevice = new AudioIn();
    qDebug() << QString::number(Pa_Initialize());
    this->params.device = Pa_GetDefaultInputDevice();
    this->params.channelCount = 1;
    this->params.sampleFormat = PA_SAMPLE_TYPE;
    this->params.suggestedLatency = Pa_GetDeviceInfo( this->params.device )->defaultLowInputLatency;
    this->params.hostApiSpecificStreamInfo = NULL;
    this->logDevice->initAudioIn(this->params, 5);
    this->logDevice->testAudioIn();
    connect(this->logDevice, &AudioIn::currentAmp, this, &MainWindow::newMaxValue);
}

MainWindow::~MainWindow()
{
    if(this->logDevice != NULL)
        delete this->logDevice;
    delete ui;
}

void MainWindow::on_LogThis_clicked()
{
    this->logDevice->maxAmplitude();
}

void MainWindow::newMaxValue(double val)
{
    ui->MaxVal->setText(QString::number(val));
}
