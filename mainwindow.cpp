#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //Init variables
    this->mono_response = "";
    this->stepper_response = "";
    this->current_Mono_Position = 0;
    ui->curWL->setText("Monochromator has been reset!");
    //load current serial port list
    QList<QSerialPortInfo> list;
    list = QSerialPortInfo::availablePorts();
    this->current_Measurement = 0;
    for(int i = 0; i < list.size(); i++)
    {
        ui->mono_Connections->addItem(list[i].portName());
        ui->stepper_connections->addItem(list[i].portName());
    }
    ui->scanProgress->setValue(0);
    this->logDevice = new AudioIn();
    //qDebug() << QString::number(Pa_Initialize());
    debug_out(QString::number(Pa_Initialize()));
    this->params.device = Pa_GetDefaultInputDevice();
    this->params.channelCount = 1;
    this->params.sampleFormat = PA_SAMPLE_TYPE;
    this->params.suggestedLatency = Pa_GetDeviceInfo( this->params.device )->defaultLowInputLatency;
    this->params.hostApiSpecificStreamInfo = NULL;
    this->logDevice->initAudioIn(this->params, 3);
    this->logDevice->testAudioIn();
    QList<QString> initCommands;
    initCommands.append("Initialize Monochromator");
    initCommands.append("Goto position");
    initCommands.append("Get current position");
    ui->mono_command->addItems(initCommands);
    ui->stepper_command->setCurrentIndex(0);
    this->current_Mono_Command = "Initialize Monochromator";
    QList<QString> initCommand;
    initCommand.append("Initialize stepper motor controller");
    initCommand.append("Home controller");
    initCommand.append("Get current position");
    initCommand.append("Move absolute");
    initCommand.append("Move relative");
    initCommand.append("Get movement time");
    ui->stepper_command->addItems(initCommand);
    ui->stepper_command->setCurrentIndex(0);
    this->current_Stepper_Command = "Initialize stepper motor controller";
    ui->min_out_of_range->setText("Minimal stepper position out of range, using smallest possible range!");
    ui->max_out_of_range->setText("Maximal stepper position out of range, using biggest possible range!");
    ui->min_out_of_range->hide();
    ui->max_out_of_range->hide();
//    ui->ResetMono->hide();
//    ui->GotoPositionButton->hide();
//    ui->curPosMono->hide();
//    ui->RelStepperButton->hide();
//    ui->AbsStepperButton->hide();
//    ui->PosStepperButton->hide();
    ui->MonoErrorLabel->hide();
    ui->stepperErrorLabel->hide();
    this->hideMonoControls(0);
    this->hideStepperControls(0);
    connect(this->logDevice, &AudioIn::currentAmp, this, &MainWindow::getCurValue);
    connect(this, &MainWindow::getNewValue, this->logDevice, &AudioIn::maxAmplitude);
    connect(this->logDevice, &AudioIn::currentSound, this, &MainWindow::getCurFFT);
    connect(this, &MainWindow::getNewFFT, this->logDevice, &AudioIn::getFFTdata);
}

MainWindow::~MainWindow()
{
    if(this->monochromator != NULL)
        delete this->monochromator;
    if(this->stepper != NULL)
        delete this->stepper;
    if(this->logDevice != NULL)
        delete this->logDevice;
    if(this->mono != NULL)
        delete this->mono;
    if(this->stepp != NULL)
        delete this->stepp;
    delete ui;
}

void MainWindow::on_connectMono_clicked()
{
    QList<QSerialPortInfo> list;
    list = QSerialPortInfo::availablePorts();
//    this->monochromator = new serial_controller(list[ui->mono_Connections->currentIndex()].portName(), 1000, 9600, 1, false, false, false, true);
//    connect(this->monochromator, &serial_controller::response, this, &MainWindow::Received_Mono_Data);
//    connect(this, &MainWindow::executeCommandMono, this->monochromator, &serial_controller::transaction);
    this->mono = new monoChrom(list[ui->mono_Connections->currentIndex()].portName());
    connect(this, &MainWindow::resetMono, this->mono, &monoChrom::resetMono);
    connect(this, &MainWindow::moveToWL, this->mono, &monoChrom::moveMonoToWL);
    connect(this, &MainWindow::getCurrentPos, this->mono, &monoChrom::getCurrentWL);
    connect(this->mono, &monoChrom::connectionError, this, &MainWindow::monoConnectionError);

    ui->ResetMono->show();
    ui->connectMono->hide();
    ui->mono_Connections->hide();
    ui->monoPortLabel->hide();
    this->hideMonoControls(1);
}

void MainWindow::monoConnectionError(bool error)
{
    if(error == false)
    {
        delete this->mono;
        this->mono = NULL;
        this->hideMonoControls(0);
        ui->ResetMono->hide();
        ui->connectMono->show();
        ui->mono_Connections->show();
        ui->monoPortLabel->show();
        ui->MonoErrorLabel->show();
        ui->MonoErrorLabel->setStyleSheet("QLabel { background-color: red; }");
    }
}

void MainWindow::on_connect_stepper_clicked()
{
    QList<QSerialPortInfo> list;
    list = QSerialPortInfo::availablePorts();
//    this->stepper = new serial_controller(list[ui->stepper_connections->currentIndex()].portName(), 1000, 57600, 1, false, false, false, false);
//    connect(this->stepper, &serial_controller::response, this, &MainWindow::Received_Stepper_Data);
//    connect(this, &MainWindow::executeCommandStepper, this->stepper, &serial_controller::transaction);
    this->stepp = new stepperM(list[ui->stepper_connections->currentIndex()].portName(), MIN, MAX);
    connect(this, &MainWindow::AbsStepper, this->stepp, &stepperM::moveAbs);
    connect(this, &MainWindow::RelStepper, this->stepp, &stepperM::moveRel);
    connect(this, &MainWindow::homeMirror, this->stepp, &stepperM::home);
    connect(this, &MainWindow::getMovementTime, this->stepp, &stepperM::getEstimatedMovementTime);
    connect(this->stepp, &stepperM::updateEstTime, this, &MainWindow::movementTimeUpdate);
    connect(this->stepp, &stepperM::curPosUpdate, this, &MainWindow::CurPosUpdate);
    connect(this->stepp, &stepperM::movementFinished, this, &MainWindow::ScanMovementStopped);
    connect(this->stepp, &stepperM::connErr, this, &MainWindow::stepperConnectionError);

    ui->connect_stepper->hide();
    ui->stepper_connections->hide();
    ui->stepperPortLabel->hide();
    this->hideStepperControls(1);
}

void MainWindow::stepperConnectionError(bool error)
{
    if(error == false)
    {
        delete this->stepp;
        this->hideStepperControls(0);
        ui->stepperErrorLabel->show();
        ui->stepperErrorLabel->setStyleSheet("QLabel { background-color: red; }");
        this->stepp = NULL;
        ui->connect_stepper->show();
        ui->stepper_connections->show();
        ui->stepperPortLabel->show();
    }
}


void MainWindow::on_Send_Data_Mono_clicked()
{
    this->mono_response.clear();
    ui->mono_result->setText("");
    if(this->mono != NULL)
    {
        if(this->current_Mono_Command == "Initialize Monochromator")
            //emit this->executeCommandMono("FFFFFF", 0);
            emit this->resetMono();
        else if(this->current_Mono_Command == "Goto position")
        {
            //this->moveMonoToPosition(ui->Mono_Value_1->text().isEmpty()?0:ui->Mono_Value_1->text().toInt());
            if(!ui->Mono_Value_1->text().isEmpty())
            {
                bool ok = false;
                ui->Mono_Value_1->text().toInt(&ok);
                if(ok)
                {
                    emit this->moveToWL(ui->Mono_Value_1->text().toInt(&ok));
                    this->current_Mono_Position = ui->Mono_Value_1->text().toInt();
                    ui->curWL->setText(QString::number(ui->Mono_Value_1->text().toInt()));
                }
            }
            //ui->curWL->setText(QString::number(ui->Mono_Value_1->text().toInt()));
            //ui->curWL->setText("");

        }
        else if(this->current_Mono_Command == "Get current position")
        {
            //qDebug() << "3800";
            debug_out("3800, not implemented yet!");
            emit this->getCurrentPos();
        }
        else
            //qDebug() << "Unknown mono command";
            debug_out("Unknown mono command");
    }
    ui->Mono_Value_1->setText("");
}

void MainWindow::CurPosUpdate(double pos)
{
    //this->StepperData.curPos/*this->current_Stepper_Position*/ = pos;
    this->StepperData.curPos = pos;
    ui->stepper_result->setText("Current position is: " + QString::number(this->StepperData.curPos/*this->current_Stepper_Position*/));
}


void MainWindow::on_Send_Data_Stepper_clicked()
{
    this->stepper_response.clear();
    ui->stepper_result->setText("");
    if(this->stepp != NULL)
    {
        if(this->current_Stepper_Command == "Initialize stepper motor controller")
        {
            //emit this->executeCommandStepper("1OR?", 0.01);
            debug_out("Init chosen!", 1);
            //qDebug() << "Init chosen!";
            this->homeStepper();
        }
        else if(this->current_Stepper_Command == "Home controller")
        {
            //emit this->executeCommandStepper("1OR?", 0);
            this->homeStepper();
        }
        else if(this->current_Stepper_Command == "Get current position")
        {
            this->StepperData.curPos/*this->current_Stepper_Position*/ = this->stepp->getCurPos();
        }
        else if(this->current_Stepper_Command == "Move absolute")
        {
            if(ui->Stepper_Value_1->text().isEmpty() == false)
            {
                bool res = true;
                if(ui->Stepper_Value_1->text().toDouble() > MAX)
                {
                    res = emit this->AbsStepper((MAX));
                    if(res)
                        this->StepperData.curPos/*this->current_Stepper_Position*/ = (MAX);
                }
                else if(ui->Stepper_Value_1->text().toDouble() >= MIN && ui->Stepper_Value_1->text().toDouble() <= MAX)
                {
                    res = emit this->AbsStepper(ui->Stepper_Value_1->text().toDouble());
                    if(res)
                        this->StepperData.curPos/*this->current_Stepper_Position*/ = ui->Stepper_Value_1->text().toDouble();
                }
                else
                {
                    res = emit this->AbsStepper(MIN);
                    if(res)
                        this->StepperData.curPos/*this->current_Stepper_Position*/ = MIN;
                }
                //bool res = emit this->AbsStepper((ui->Stepper_Value_1->text().toDouble() > this->stepper_max_limit?this->stepper_max_limit:ui->Stepper_Value_1->text().toDouble()));
                if(res == true)
                    debug_out("Success!");
                    //qDebug() << "Success!";
                else
                    debug_out("Failure!");
                    //qDebug() << "Failure!";
            }

        }
        else if(this->current_Stepper_Command == "Move relative")
        {
            if(ui->Stepper_Value_1->text().isEmpty() == false)
            {
                bool res = true;
                if(ui->Stepper_Value_1->text().toDouble() > 0)
                {
                    if(this->StepperData.curPos/*this->current_Stepper_Position*/ + ui->Stepper_Value_1->text().toDouble() > MAX)
                    {
                        res = emit this->RelStepper(MAX - this->StepperData.curPos/*this->current_Stepper_Position*/);
                        this->StepperData.curPos/*this->current_Stepper_Position*/ = MAX;
                    }
                    else
                    {
                        res = emit this->RelStepper(ui->Stepper_Value_1->text().toDouble());
                        this->StepperData.curPos/*this->current_Stepper_Position*/ += ui->Stepper_Value_1->text().toDouble();
                    }
                }
                else
                {
                    if(this->StepperData.curPos/*this->current_Stepper_Position*/ + ui->Stepper_Value_1->text().toDouble() < MIN)
                    {
                        res = emit this->RelStepper(MIN - this->StepperData.curPos/*this->current_Stepper_Position*/);
                        this->StepperData.curPos/*this->current_Stepper_Position*/ = MIN;
                    }
                    else
                    {
                        res = emit this->RelStepper(ui->Stepper_Value_1->text().toDouble());
                        this->StepperData.curPos/*this->current_Stepper_Position*/ += ui->Stepper_Value_1->text().toDouble();
                    }

                }
                //bool res = emit this->RelStepper(((ui->Stepper_Value_1->text().toDouble() > 0)?((ui->Stepper_Value_1->text().toDouble() + this->StepperData.curPos/*this->current_Stepper_Position*/ > this->stepper_max_limit?(this->stepper_max_limit-ui->Stepper_Value_1->text().toDouble()):ui->Stepper_Value_1->text()):(this->StepperData.curPos/*this->current_Stepper_Position*/ - ui->Stepper_Value_1->text().toDouble() < this->stepper_min_limit)?this->StepperData.curPos/*this->current_Stepper_Position*/ - this->stepper_min_limit:ui->Stepper_Value_1->text().toDouble())));

                if(res == true)
                    debug_out("Success!");
                    //qDebug() << "Success!";
                else
                    debug_out("Failure!");
                    //qDebug() << "Failure!";
            }
        }
        else if(this->current_Stepper_Command == "Get movement time")
        {
            this->getEstimatedMovementTime(ui->Stepper_Value_1->text().isEmpty()?0:ui->Stepper_Value_1->text().toDouble());
        }
        else
            debug_out("Unknown command!");
            //qDebug() << "Unknown command!";
    }
    ui->Stepper_Value_1->setText("");
}


void MainWindow::getEstimatedMovementTime(double relPos)
{
    //emit this->executeCommandStepper("1PT"+QString::number(relPos), 0);
    emit this->getMovementTime(relPos);
}

void MainWindow::movementTimeUpdate(double Time)
{
    ui->stepper_result->setText("Estimated movement time is: " + QString::number(Time) + "s");
}

void MainWindow::Received_Mono_Data(QString &data)
{
    this->mono_response += data;
    QByteArray data_local = data.toLocal8Bit();
    QVector<int> data_int;
    int WL = -1;
    for(int i = 0; i < data_local.length(); i++)
    {
        data_int.push_back((int)data_local[i]);
        //qDebug() << data_int;
        QString tmp;
        for(int i = 0; i < data_int.size(); i++)
            tmp += QString::number(data_int[i]) + " ";
        debug_out(tmp);
    }
    if(data_int.last() == 24 && data_int[data_int.length()-2] == 17)
        debug_out("Everything ok!");
        //qDebug() << "Everything ok";
    if(data_int.size() >= 4)
    {
        WL = 0;
        QVector<int> values;
        for(int i = 0; i < data_int.size()-1; i++)
            if(data_int[i+1] != 24)//Can lead to a buffer overflow, better solution?
                values.push_back((data_int[i] > 0)?data_int[i]:128+data_int[i]);
        for(int i = 0; i < values.size(); i++)
        {
            debug_out("I: " + QString::number(i) + " & value: " + QString::number(values[i]));
            //qDebug() << "I: " << i << " & value: " << values[i];
            //qDebug() << "Current value is: " << QString::number((values[i] < 15)?values[i]*pow(16, values.size() - (i+1)):(values[i] < 65)?(values[i] - 32)*pow(16, values.size() - (i+1)):(values[i]-56)*pow(16, values.size() - (i+1))) << " at " << i;
            debug_out("Current value is: " + QString::number((values[i] < 15)?values[i]*pow(16, values.size() - (i+1)):(values[i] < 65)?(values[i] - 32)*pow(16, values.size() - (i+1)):(values[i]-56)*pow(16, values.size() - (i+1))) + " at " + QString::number(i));
            WL += (values[i] < 15)?values[i]*pow(16, values.size() - (i+1)):(values[i] < 65)?(values[i] - 32)*pow(16, values.size() - (i+1)):(values[i]-56)*pow(16, values.size() - (i+1));
        }
    }
    if(WL != -1)
        ui->mono_result->setText(QString::number(WL));
}

void MainWindow::Received_Stepper_Data(QString &data)
{
    this->stepper_response.clear();
    this->stepper_response = data;
    data.clear();
    ui->stepper_result->setText(this->stepper_response);
    if(this->stepper_response.contains("PT"))
    {
        QList<QString> response = this->stepper_response.split("PT");
        this->StepperData.estMovementTime /*this->movementTime*/ = response.length() == 2?response[1].toDouble():0;
    }
    else if(this->stepper_response.contains("TP"))
    {
        QList<QString> response = this->stepper_response.split("TP");
        this->StepperData.curPos/*this->current_Stepper_Position*/ = response.length() == 2?response[1].toDouble():0;
    }
}

void MainWindow::on_stepper_command_currentIndexChanged(const QString &arg1)
{
    this->current_Stepper_Command = arg1;
}

void MainWindow::on_mono_command_currentIndexChanged(const QString &arg1)
{
    this->current_Mono_Command = arg1;
}

void MainWindow::cleanPlot()
{
    MainWindow::Grid.attach(ui->resultPlot);
    MainWindow::Curve.attach(NULL);
    MainWindow::Curve.setTitle("Dispersion curve");
    MainWindow::Curve.setRenderHint(QwtPlotItem::RenderAntialiased, true);

    MainWindow::pen.setStyle(Qt::SolidLine);
    MainWindow::pen.setWidth(3);
    MainWindow::pen.setBrush(Qt::blue);
    MainWindow::pen.setCapStyle(Qt::RoundCap);
    MainWindow::pen.setJoinStyle(Qt::RoundJoin);

    MainWindow::Curve.setPen(MainWindow::pen);
    ui->resultPlot->setAxisTitle(QwtPlot::yLeft, "");
    ui->resultPlot->setAxisTitle(QwtPlot::xBottom, "Position [mm]");
    QVector<double> x;
    QVector<double> y;
    this->plotData.clear();
    vectorToMap(this->cleanValues, this->plotData);
    x = QVector<double>::fromList(this->plotData.keys());
    this->Curve.setSamples(x, y);
    this->Curve.attach(ui->resultPlot);
    ui->resultPlot->updateAxes();
    ui->resultPlot->show();
    ui->resultPlot->replot();
}

void MainWindow::replot()
{
    MainWindow::Grid.attach(ui->resultPlot);
    MainWindow::Curve.attach(NULL);
    MainWindow::Curve.setTitle("Dispersion curve");
    MainWindow::Curve.setRenderHint(QwtPlotItem::RenderAntialiased, true);

    MainWindow::pen.setStyle(Qt::SolidLine);
    MainWindow::pen.setWidth(3);
    MainWindow::pen.setBrush(Qt::blue);
    MainWindow::pen.setCapStyle(Qt::RoundCap);
    MainWindow::pen.setJoinStyle(Qt::RoundJoin);

    MainWindow::Curve.setPen(MainWindow::pen);
    ui->resultPlot->setAxisTitle(QwtPlot::yLeft, "");
    ui->resultPlot->setAxisTitle(QwtPlot::xBottom, "Position [mm]");
    QVector<double> x;
    QVector<double> y;
    this->plotData.clear();
    //For debug:
    for(int i = 0; i < this->dispValues.length(); i++)
    {
        debug_out("DispVal[" + QString::number(i) + "] is: " + QString::number(this->dispValues[i].first) + ", " + QString::number(this->dispValues[i].second) + "\n");
        //qDebug() << "DispVal[" << i << "] is: " << QString::number(this->dispValues[i].first) << ", " << QString::number(this->dispValues[i].second) << '\n';
    }
    if(this->dispValues.isEmpty() == false)
    {
        vectorToMap(this->dispValues, this->plotData);
        x = QVector<double>::fromList(this->plotData.keys());
        debug_out("First element of plotData is: " + QString::number(this->plotData.first()));
        //qDebug() << "First element of plotData is: " << this->plotData.first();
        debug_out("Length of plotData-values: " + QString::number(this->plotData.values().length()));
        //qDebug() << "Length of plotData-values: " << this->plotData.values().length();
        debug_out("Length of plotData-Keys: " + QString::number(this->plotData.keys().length()));
        //qDebug() << "Length of plotData-Keys: " << this->plotData.keys().length();
        debug_out("First value: " + QString::number(this->plotData.values().first()));
        //qDebug() << "First value: " << this->plotData.values().first();
        y = QVector<double>::fromList(this->plotData.values());
        this->Curve.setSamples(x, y);
        this->Curve.attach(ui->resultPlot);
        ui->resultPlot->updateAxes();
        ui->resultPlot->show();
        ui->resultPlot->replot();
    }
    else
        debug_out("Data size: " + QString::number(this->dispValues.size()));
        //qDebug() << "Data size: " << this->dispValues.size();
}

void MainWindow::homeStepper()
{
//    this->getEstimatedMovementTime(this->StepperData.curPos/*this->current_Stepper_Position*/);
//    QThread::msleep(100);
//    this->StepperData.curPos/*this->current_Stepper_Position*/ = 0;
//    emit this->executeCommandStepper("1OR?", 0);
//    QThread::sleep(this->StepperData.estMovementTime/*this->movementTime*/);
    debug_out("In main: Home()");
    //qDebug() << "In main: Home()";
    emit this->homeMirror();

}

void MainWindow::on_startScan_clicked()
{
    if(this->stepp == NULL || this->mono == NULL)
    {
        debug_out("One or both of the controllers are not connected, exiting!");
        return;
    }
    //Determine time to target
    this->scanRun = true;
    this->current_Measurement++;
    this->StepperData.estMovementTime/*this->movementTime*/ = 0;
    this->dispValues.clear();
//    this->getEstimatedMovementTime(fabs(this->StepperData.curPos/*this->current_Stepper_Position*/-this->stepper_min_limit));
//    qDebug() << "Movement time for distance " << this->StepperData.curPos/*this->current_Stepper_Position*/-this->stepper_min_limit << " is: " << this->StepperData.estMovementTime/*this->movementTime*/;
//    QThread::msleep(100);
    double min = MIN, max = MAX;
    if(ui->minPos->text().toDouble() > ui->maxPos->text().toDouble())
    {
        double tmp;
        tmp = ui->minPos->text().toDouble();
        ui->minPos->setText(QString::number(ui->maxPos->text().toDouble()));
        ui->maxPos->setText(QString::number(tmp));
    }
    if(ui->minPos->text().toDouble() < min)
    {
        ui->min_out_of_range->show();
        ui->min_out_of_range->setStyleSheet("QLabel { background-color: red; }");
    }
    else
        ui->min_out_of_range->hide();
    if(ui->maxPos->text().toDouble() > max)
    {
        ui->max_out_of_range->show();
        ui->max_out_of_range->setStyleSheet("QLabel { background-color: red; }");
    }
    else
        ui->max_out_of_range->hide();
    double scan_min_pos = (ui->minPos->text().isEmpty()?min:(ui->minPos->text().toDouble() <= min || ui->minPos->text().toDouble() >= max)?min:ui->minPos->text().toDouble());
    double scan_max_pos = (ui->maxPos->text().isEmpty()?max:(ui->maxPos->text().toDouble() >= max || ui->maxPos->text().toDouble() <= min)?max:ui->maxPos->text().toDouble());
    int scan_steps = ui->num_steps->text().isEmpty()?500:ui->num_steps->text().toInt();
    this->ScanPosData.refill(scan_min_pos, scan_max_pos, scan_steps, this->current_Mono_Position);
    //this->step_size = (double)(this->stepper_max_limit-this->stepper_min_limit)/this->steps;//100 steps
    ui->size_steps->setText(QString::number(this->ScanPosData.stepsize));
    debug_out("Running a short scan from " + QString::number(this->ScanPosData.start) + " to " + QString::number(this->ScanPosData.stop) + " with " + QString::number(this->ScanPosData.steps) + " at the wavelength of " + QString::number(this->ScanPosData.waveLenght), 1);
    emit this->moveStepperToAbsPosition(this->ScanPosData.start);
    ui->startScan->hide();
    ui->MovStopped->hide();
    ui->minPos->setReadOnly(true);
    ui->maxPos->setReadOnly(true);
}

void MainWindow::moveMonoToPosition(int pos)
{
//    int value;
//    if(pos == 0 || pos == this->current_Mono_Position)
//        value = 0;
//    else
//        value = pos;
//    QString wavelength;
//    this->current_Mono_Position = value;
//    if(QString::number((qlonglong)value, 16).length() == 3)
//        wavelength = "0" + QString::number((qlonglong)value, 16);
//    else
//        wavelength = QString::number((qlonglong)value, 16);
//    //emit this->executeCommandMono("10" + wavelength, 0);
    emit this->moveToWL(pos);
}

void MainWindow::moveStepperToAbsPosition(double pos)
{
//    double newPosition = 0;
//    newPosition = pos;
//    if((newPosition >= this->stepper_min_limit) && (newPosition <= this->stepper_max_limit))
//    {
//        //emit this->executeCommandStepper("1PA" + QString::number(newPosition), 0);
//        this->StepperData.curPos/*this->current_Stepper_Position*/ = newPosition;
//    }
    emit this->AbsStepper(pos);
}

void MainWindow::moveStepperToRelPosition(double pos)
{
//    double newPosition = 0;
//    newPosition = pos;
//    if(this->StepperData.curPos/*this->current_Stepper_Position*/+newPosition <= this->stepper_max_limit && this->StepperData.curPos/*this->current_Stepper_Position*/ + newPosition >= this->stepper_min_limit)
//    {
//        this->StepperData.curPos/*this->current_Stepper_Position*/ = newPosition;
//        //emit this->executeCommandStepper("1PR" + QString::number(newPosition), 0);
//    }
    emit this->RelStepper(pos);
}

void MainWindow::ScanMovementStopped()
{
    if(this->scanRun)
    {
        debug_out("Get next value!");
        //qDebug() << "Get next value!";
        this->ScanPosData.curStep /*this->curStep*/ += this->ScanPosData.stepsize;//this->step_size;
        emit this->getNewValue();
        if(ui->fftCheckBox->isChecked())
            emit this->getNewFFT();
    }
    else if(this->multiAqu)
        this->doFullScan();
    debug_out("Scan stopped!");
    //qDebug() << "Scan stopped!";
}

void MainWindow::getCurValue(double val)
{
    //this->getMaxValue(val);
    debug_out("Got new value!");
    //qDebug() << "Got new value!";
    this->dispValues.push_back(qMakePair(this->StepperData.curPos/*this->current_Stepper_Position*/, val));
    if(this->StepperData.curPos/*this->current_Stepper_Position*/ + this->ScanPosData.stepsize /*this->stepper_max_limit*/ < this->ScanPosData.stop /*this->stepper_max_limit*/)
    {
        //this->moveStepperToAbsPosition(this->StepperData.curPos/*this->current_Stepper_Position*/ + this->step_size);
        //qDebug() << "Stepper moving, step " << this->curStep << ", to position " << this->StepperData.curPos/*this->current_Stepper_Position*/ << " with a stepsize of " << this->step_size << "!";
        debug_out("Stepper moving, step " + QString::number(this->ScanPosData.curStep /*this->curStep*/) + ", to position " + QString::number(this->StepperData.curPos/*this->current_Stepper_Position*/) + " with a stepsize of " + QString::number(this->ScanPosData.stepsize /*this->stepper_max_limit*/) + "!", 1);
        this->ScanPosData.curStep++;//nthis->curStep++;
        //ui->scanProgress->setValue((curStep)/this->steps);
        ui->scanProgress->setValue(this->ScanPosData.curStep/this->ScanPosData.steps);
        this->StepperData.curPos/*this->current_Stepper_Position*/ += this->ScanPosData.stepsize /*this->stepper_max_limit*/;
        double progressValue = (double)(this->StepperData.curPos/*this->current_Stepper_Position*/ - this->ScanPosData.start)/(double)(this->ScanPosData.stop /*this->stepper_max_limit*/ - this->ScanPosData.start);
        ui->scanProgress->setValue((int)(progressValue));
        emit this->moveStepperToAbsPosition(this->StepperData.curPos/*this->current_Stepper_Position*/);

        //this->getNewValue();
    }
    else
    {

        this->write_unformatted_file(this->dispValues, this->fileName);
        debug_out("Scan finished, moving back to first position!");
        //qDebug() << "Scan finished, moving back to first position!";
        this->moveStepperToAbsPosition(this->ScanPosData.start /*this->stepper_min_limit*/);
        ui->scanProgress->setValue(0);
        if(this->multiAqu == false)
        {
            ui->minPos->setReadOnly(false);
            ui->maxPos->setReadOnly(false);
            ui->startScan->show();
            ui->minPos->setText("");
            ui->maxPos->setText("");
            ui->num_steps->setText("");
            ui->startValue->setText("");
            ui->stopValue->setText("");
            //this->stepper_min_limit = MIN;
            //this->stepper_max_limit = MAX;
            this->ScanPosData.clear();
        }
        //this->step_size = 0;
        this->scanRun = false;
        //this->curStep = 0;
    }
    this->replot();
}

void MainWindow::getCurFFT(QVector<double> val)
{
    QString filename = QString::number(this->current_Mono_Position) + QString::number(this->StepperData.curPos/*this->current_Stepper_Position*/) + ".txt";
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly))
    {
        debug_out(file.errorString(), 1);
        return;
    }
    QTextStream out(&file);
    for(int i = 0; i < val.size(); i++)
        out << i << '\t' << val[i] << '\n';
    file.close();
}

void MainWindow::write_unformatted_file(const QVector<QPair<int, QPair<double, double> > > &Data/*const QMap<double, double> &Data*/, QString fileName = "DispersionValues")
{
    QDir dir;
    fileName += ".txt";
    fileName = dir.absolutePath() + "/" + fileName;
    debug_out("Writing into file " + fileName);
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly)) {
        debug_out(file.errorString());
        //qDebug() << file.errorString();
        return;
    }
    QTextStream out(&file);
    for(int i = 0; i < Data.size(); i++)
    {
        out << Data[i].first << '\t' << Data[i].second.first << '\t' << Data[i].second.second << '\n';
    }

    file.close();
}

void MainWindow::write_unformatted_file(const QVector<QPair<double, double> > &Data/*const QMap<double, double> &Data*/, QString fileName)
{
    fileName += ".txt";
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly)) {
        debug_out(file.errorString());
        //qDebug() << file.errorString();
        return;
    }
    QTextStream out(&file);
    for(int i = 0; i < Data.size(); i++)
    {
        out << Data[i].first << '\t' << Data[i].second << '\n';
    }

    file.close();
}

void MainWindow::init_scan()
{
//    this->step_size = (double)(this->stepper_max_limit-this->stepper_min_limit)/(double)num_steps;
//    this->moveStepperToAbsPosition(this->stepper_min_limit);
//    QThread::msleep(100);
//    qDebug() << "Make sure that the stepper is not moving anymore!";
//    this->curStep = 0;
}

QPair<double, double> MainWindow::getMaxValue(void)
{
    double maxVal = 0;
    double curPos = 0;
    for(int i = 0; i < this->dispValues.length(); i++)
    {
        if(this->dispValues[i].second > maxVal)
            curPos = this->dispValues[i].first;
    }
    return qMakePair(curPos, maxVal);
}

void MainWindow::on_MovStopped_clicked()
{
    this->ScanPosData.curStep /*this->curStep*/ += this->ScanPosData.stepsize;//this->step_size;
    emit this->getNewValue();
    if(ui->fftCheckBox->isChecked())
        emit this->getNewFFT();
}

void MainWindow::on_FullScan_clicked()
{
    if(this->stepp == NULL || this->mono == NULL)
    {
        debug_out("One or both of the controllers are not connected, exiting!");
        return;
    }
    if(ui->startValue->text().isEmpty() || ui->stopValue->text().isEmpty() || ui->startValue->text().toInt() >= ui->stopValue->text().toInt())
        return;
    if(ui->minPos->text().isEmpty() || ui->maxPos->text().isEmpty())
        return;
    if(this->current_Mono_Position != ui->startValue->text().toInt())
        this->moveMonoToPosition(ui->startValue->text().toInt());
    QThread::msleep(1000);
    this->multiAqu = true;
    this->dispValues.clear();
    this->on_startScan_clicked();
    this->fileName = ui->startValue->text();
    //this->wlSteps = 0;
}

void MainWindow::doFullScan()
{
    if(this->stepp == NULL || this->mono == NULL)
    {
        debug_out("One or both of the controllers are not connected, exiting!");
        return;
    }
    if(this->ScanPosData.wavelenghtSteps + ui->startValue->text().toInt() < ui->stopValue->text().toInt())
    {
        debug_out("Scan " + QString::number(this->ScanPosData.wavelenghtSteps) + " finished!");
        //qDebug() << "Scan " << this->wlSteps << " finished!";
        this->DispResults.push_back(qMakePair(this->current_Mono_Position, this->getMaxValue()));
        this->moveMonoToPosition(this->current_Mono_Position + this->ScanPosData.wavelenghtSteps + 1);
        QThread::sleep(1);
        this->ScanPosData.wavelenghtSteps+=1;
        this->fileName = QString::number(ui->startValue->text().toInt() + '_' + this->ScanPosData.wavelenghtSteps);
        this->on_startScan_clicked();
    }
    else
    {
        this->multiAqu = false;
        this->write_unformatted_file(this->DispResults);
        //this->wlSteps = 0;
        //this->stepper_max_limit = MAX;
        //this->stepper_min_limit = MIN;
        this->ScanPosData.clear();
        ui->startScan->show();
    }
}

void MainWindow::on_ResetMono_clicked()
{
    emit this->resetMono();
    ui->curPosMono->show();
    this->current_Mono_Position = 0;
    ui->curWL->setText("Monochromator reset!");
    ui->ResetMono->hide();
}

void MainWindow::on_Mono_Value_1_textChanged(const QString &arg1)
{
    UNUSED(arg1);
    bool ok;
    if(!ui->Mono_Value_1->text().isEmpty() && ui->Mono_Value_1->text().toInt(&ok))
        if(ok)
            ui->GotoPositionButton->show();
        else
            ui->GotoPositionButton->hide();
    else
        ui->GotoPositionButton->hide();
}

void MainWindow::on_GotoPositionButton_clicked()
{
    emit this->resetMono();
    //Sleep?
    emit this->moveMonoToPosition(ui->Mono_Value_1->text().toInt());
    this->current_Mono_Position = ui->Mono_Value_1->text().toInt();
    ui->curWL->setText(QString::number(this->current_Mono_Position));
}

void MainWindow::on_fftCheckBox_clicked()
{

}

void MainWindow::on_homeStepperButton_clicked()
{
    emit this->homeStepper();
}


void MainWindow::on_PosStepperButton_clicked()
{
    if(this->stepp->getCurPos() == false)
        ui->stepper_result->setText("Problem with aquiring data from stepper, please verify connection!");

}

void MainWindow::on_AbsStepperButton_clicked()
{
    if(ui->Stepper_Value_1->text().isEmpty() == false)
    {
        bool res = true;
        if(ui->Stepper_Value_1->text().toDouble() > MAX)
        {
            res = emit this->AbsStepper((MAX));
            if(res)
                this->StepperData.curPos/*this->current_Stepper_Position*/ = (MAX);
        }
        else if(ui->Stepper_Value_1->text().toDouble() >= MIN && ui->Stepper_Value_1->text().toDouble() <= MAX)
        {
            res = emit this->AbsStepper(ui->Stepper_Value_1->text().toDouble());
            if(res)
                this->StepperData.curPos/*this->current_Stepper_Position*/ = ui->Stepper_Value_1->text().toDouble();
        }
        else
        {
            res = emit this->AbsStepper(MIN);
            if(res)
                this->StepperData.curPos/*this->current_Stepper_Position*/ = MIN;
        }
        if(res == true)
            debug_out("Success!");
        else
            debug_out("Failure!");
    }
}

void MainWindow::on_RelStepperButton_clicked()
{
    if(ui->Stepper_Value_1->text().isEmpty() == false)
    {
        bool res = true;
        if(ui->Stepper_Value_1->text().toDouble() > 0)
        {
            if(this->StepperData.curPos/*this->current_Stepper_Position*/ + ui->Stepper_Value_1->text().toDouble() > MAX)
            {
                res = emit this->RelStepper(MAX - this->StepperData.curPos/*this->current_Stepper_Position*/);
                this->StepperData.curPos/*this->current_Stepper_Position*/ = MAX;
            }
            else
            {
                res = emit this->RelStepper(ui->Stepper_Value_1->text().toDouble());
                this->StepperData.curPos/*this->current_Stepper_Position*/ += ui->Stepper_Value_1->text().toDouble();
            }
        }
        else
        {
            if(this->StepperData.curPos/*this->current_Stepper_Position*/ + ui->Stepper_Value_1->text().toDouble() < MIN)
            {
                res = emit this->RelStepper(MIN - this->StepperData.curPos/*this->current_Stepper_Position*/);
                this->StepperData.curPos/*this->current_Stepper_Position*/ = MIN;
            }
            else
            {
                res = emit this->RelStepper(ui->Stepper_Value_1->text().toDouble());
                this->StepperData.curPos/*this->current_Stepper_Position*/ += ui->Stepper_Value_1->text().toDouble();
            }

        }
        //bool res = emit this->RelStepper(((ui->Stepper_Value_1->text().toDouble() > 0)?((ui->Stepper_Value_1->text().toDouble() + this->StepperData.curPos/*this->current_Stepper_Position*/ > this->stepper_max_limit?(this->stepper_max_limit-ui->Stepper_Value_1->text().toDouble()):ui->Stepper_Value_1->text()):(this->StepperData.curPos/*this->current_Stepper_Position*/ - ui->Stepper_Value_1->text().toDouble() < this->stepper_min_limit)?this->StepperData.curPos/*this->current_Stepper_Position*/ - this->stepper_min_limit:ui->Stepper_Value_1->text().toDouble())));

        if(res == true)
            debug_out("Success!");
        //qDebug() << "Success!";
        else
            debug_out("Failure!");
        //qDebug() << "Failure!";
    }
}

void MainWindow::on_Stepper_Value_1_textEdited(const QString &arg1)
{
    if(arg1.isEmpty())
    {
        ui->RelStepperButton->hide();
        ui->AbsStepperButton->hide();
    }
    else
    {
        ui->RelStepperButton->show();
        ui->AbsStepperButton->show();
    }
}

void MainWindow::hideStepperControls(int level)
{
    switch(level)
    {
    case 0://Lowest level, everything gone before connecting
        ui->AbsStepperButton->hide();
        ui->RelStepperButton->hide();
        ui->stepper_command->hide();
        ui->label_6->hide();
        ui->homeStepperButton->hide();
        ui->PosStepperButton->hide();
        ui->label_8->hide();
        ui->Stepper_Value_1->hide();
        ui->label_5->hide();
        ui->stepper_result->hide();
        ui->Send_Data_Stepper->hide();
        ui->connect_stepper->show();
        ui->stepperPortLabel->show();
        ui->stepper_connections->show();
        break;
    case 1://Connection positive, no command send yet
        ui->AbsStepperButton->hide();
        ui->RelStepperButton->hide();
        ui->stepper_command->show();
        ui->label_6->show();
        ui->homeStepperButton->show();
        ui->PosStepperButton->show();
        ui->label_8->show();
        ui->Stepper_Value_1->show();
        ui->label_5->show();
        ui->stepper_result->show();
        ui->Send_Data_Stepper->show();
        ui->connect_stepper->hide();
        ui->stepperPortLabel->hide();
        ui->stepper_connections->hide();
        ui->stepperErrorLabel->hide();
        break;
    case 2://Initcommand send, normal working case
        ui->AbsStepperButton->show();
        ui->RelStepperButton->show();
        ui->stepper_command->show();
        ui->label_6->show();
        ui->homeStepperButton->show();
        ui->PosStepperButton->show();
        ui->label_8->show();
        ui->Stepper_Value_1->show();
        ui->label_5->show();
        ui->stepper_result->show();
        ui->Send_Data_Stepper->show();
        ui->connect_stepper->hide();
        ui->stepperPortLabel->hide();
        ui->stepper_connections->hide();
        break;
    default://Not sure where I am
        ui->AbsStepperButton->hide();
        ui->RelStepperButton->hide();
        ui->stepper_command->hide();
        ui->label_6->hide();
        ui->homeStepperButton->hide();
        ui->PosStepperButton->hide();
        ui->label_8->hide();
        ui->Stepper_Value_1->hide();
        ui->label_5->hide();
        ui->stepper_result->hide();
        ui->Send_Data_Stepper->hide();
        ui->connect_stepper->show();
        ui->stepperPortLabel->show();
        ui->stepper_connections->show();
        break;
    }

}

void MainWindow::hideMonoControls(int level)
{
    switch(level)
    {
    case 0://Before connection
        ui->connectMono->show();
        ui->monoConnections->show();
        ui->monoPortLabel->show();
        ui->label_4->hide();
        ui->mono_command->hide();
        ui->label_9->hide();
        ui->mono_result->hide();
        ui->Mono_Value_1->hide();
        ui->Send_Data_Mono->hide();
        ui->ResetMono->hide();
        ui->GotoPositionButton->hide();
        ui->curPosMono->hide();
        ui->label_7->hide();

        break;
    case 1://After connection, before init
        ui->connectMono->hide();
        ui->monoConnections->hide();
        ui->monoPortLabel->hide();
        ui->label_4->show();
        ui->mono_command->show();
        ui->label_9->hide();
        ui->mono_result->show();
        ui->Mono_Value_1->hide();
        ui->Send_Data_Mono->show();
        ui->ResetMono->show();
        ui->GotoPositionButton->hide();
        ui->curPosMono->show();
        ui->label_7->show();
        ui->MonoErrorLabel->hide();
        break;
    case 2://Normal operation mode
        ui->connectMono->hide();
        ui->monoConnections->hide();
        ui->monoPortLabel->hide();
        ui->label_4->show();
        ui->mono_command->show();
        ui->label_9->show();
        ui->mono_result->show();
        ui->Mono_Value_1->show();
        ui->Send_Data_Mono->show();
        ui->ResetMono->show();
        ui->GotoPositionButton->show();
        ui->curPosMono->show();
        ui->label_7->show();
        break;
    default:
        ui->connectMono->show();
        ui->monoConnections->show();
        ui->monoPortLabel->show();
        ui->label_4->hide();
        ui->mono_command->hide();
        ui->label_9->hide();
        ui->mono_result->hide();
        ui->Mono_Value_1->hide();
        ui->Send_Data_Mono->hide();
        ui->ResetMono->hide();
        ui->GotoPositionButton->hide();
        ui->curPosMono->hide();
        ui->label_7->hide();
        break;

    }
}
