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
    ui->min_out_of_range->setText("Minimal stepper position out of range, using smallest possible range!");
    ui->max_out_of_range->setText("Maximal stepper position out of range, using biggest possible range!");
    ui->min_out_of_range->hide();
    ui->max_out_of_range->hide();
    connect(this->logDevice, &AudioIn::currentAmp, this, &MainWindow::getCurValue);
    connect(this, &MainWindow::getNewValue, this->logDevice, &AudioIn::maxAmplitude);
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
    QList<QString> initCommands;
    initCommands.append("Initialize Monochromator");
    initCommands.append("Goto position");
    initCommands.append("Get current position");
    ui->mono_command->addItems(initCommands);
    ui->stepper_command->setCurrentIndex(0);
    this->current_Mono_Command = "Initialize Monochromator";
    ui->connectMono->hide();
}

void MainWindow::on_connect_stepper_clicked()
{
    QList<QSerialPortInfo> list;
    list = QSerialPortInfo::availablePorts();
//    this->stepper = new serial_controller(list[ui->stepper_connections->currentIndex()].portName(), 1000, 57600, 1, false, false, false, false);
//    connect(this->stepper, &serial_controller::response, this, &MainWindow::Received_Stepper_Data);
//    connect(this, &MainWindow::executeCommandStepper, this->stepper, &serial_controller::transaction);
    this->stepp = new stepperM(list[ui->stepper_connections->currentIndex()].portName());
    connect(this, &MainWindow::AbsStepper, this->stepp, &stepperM::moveAbs);
    connect(this, &MainWindow::RelStepper, this->stepp, &stepperM::moveRel);
    connect(this, &MainWindow::homeMirror, this->stepp, &stepperM::home);
    connect(this, &MainWindow::getMovementTime, this->stepp, &stepperM::getEstimatedMovementTime);
    connect(this->stepp, &stepperM::updateEstTime, this, &MainWindow::movementTimeUpdate);
    connect(this->stepp, &stepperM::curPosUpdate, this, &MainWindow::CurPosUpdate);
    connect(this->stepp, &stepperM::movementFinished, this, &MainWindow::ScanMovementStopped);
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
    ui->connect_stepper->hide();
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
            emit this->moveToWL(ui->Mono_Value_1->text().isEmpty()?0:ui->Mono_Value_1->text().toInt());
        }
        else if(this->current_Mono_Command == "Get current position")
        {
            //qDebug() << "3800";
            debug_out("3800");
            //emit this->executeCommandMono("3800", 0);
        }
        else
            //qDebug() << "Unknown mono command";
            debug_out("Unknown mono command");
    }
    ui->Mono_Value_1->setText("");
}

void MainWindow::CurPosUpdate(double pos)
{
    this->current_Stepper_Position = pos;
    ui->stepper_result->setText("Current position is: " + QString::number(this->current_Stepper_Position));
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
            this->current_Stepper_Position = this->stepp->getCurPos();
        }
        else if(this->current_Stepper_Command == "Move absolute")
        {
            if(ui->Stepper_Value_1->text().isEmpty() == false)
            {
                bool res = true;
                if(ui->Stepper_Value_1->text().toDouble() > this->stepper_max_limit)
                {
                    res = emit this->AbsStepper(this->stepper_max_limit);
                    this->current_Stepper_Position = this->stepper_max_limit;
                }
                else if(ui->Stepper_Value_1->text().toDouble() >= this->stepper_min_limit && ui->Stepper_Value_1->text().toDouble() <= this->stepper_min_limit)
                {
                    res = emit this->AbsStepper(ui->Stepper_Value_1->text().toDouble());
                    this->current_Stepper_Position = ui->Stepper_Value_1->text().toDouble();
                }
                else
                {
                    res = emit this->AbsStepper(this->stepper_min_limit);
                    this->current_Stepper_Position = this->stepper_min_limit;
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
                    if(this->current_Stepper_Position + ui->Stepper_Value_1->text().toDouble() > this->stepper_max_limit)
                    {
                        res = emit this->RelStepper(this->stepper_max_limit - this->current_Stepper_Position);
                        this->current_Stepper_Position = this->stepper_max_limit;
                    }
                    else
                    {
                        res = emit this->RelStepper(ui->Stepper_Value_1->text().toDouble());
                        this->current_Stepper_Position += ui->Stepper_Value_1->text().toDouble();
                    }
                }
                else
                {
                    if(this->current_Stepper_Position - ui->Stepper_Value_1->text().toDouble() < this->stepper_min_limit)
                    {
                        res = emit this->RelStepper(this->stepper_min_limit - this->current_Stepper_Position);
                        this->current_Stepper_Position = this->stepper_min_limit;
                    }
                    else
                    {
                        res = emit this->RelStepper(ui->Stepper_Value_1->text().toDouble());
                        this->current_Stepper_Position += ui->Stepper_Value_1->text().toDouble();
                    }

                }
                //bool res = emit this->RelStepper(((ui->Stepper_Value_1->text().toDouble() > 0)?((ui->Stepper_Value_1->text().toDouble() + this->current_Stepper_Position > this->stepper_max_limit?(this->stepper_max_limit-ui->Stepper_Value_1->text().toDouble()):ui->Stepper_Value_1->text()):(this->current_Stepper_Position - ui->Stepper_Value_1->text().toDouble() < this->stepper_min_limit)?this->current_Stepper_Position - this->stepper_min_limit:ui->Stepper_Value_1->text().toDouble())));

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
        this->movementTime = response.length() == 2?response[1].toDouble():0;
    }
    else if(this->stepper_response.contains("TP"))
    {
        QList<QString> response = this->stepper_response.split("TP");
        this->current_Stepper_Position = response.length() == 2?response[1].toDouble():0;
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
//    this->getEstimatedMovementTime(this->current_Stepper_Position);
//    QThread::msleep(100);
//    this->current_Stepper_Position = 0;
//    emit this->executeCommandStepper("1OR?", 0);
//    QThread::sleep(this->movementTime);
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
    this->movementTime = 0;
    this->dispValues.clear();
//    this->getEstimatedMovementTime(fabs(this->current_Stepper_Position-this->stepper_min_limit));
//    qDebug() << "Movement time for distance " << this->current_Stepper_Position-this->stepper_min_limit << " is: " << this->movementTime;
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
    this->stepper_min_limit = (ui->minPos->text().isEmpty()?min:(ui->minPos->text().toDouble() <= min || ui->minPos->text().toDouble() >= max)?min:ui->minPos->text().toDouble());
    this->stepper_max_limit = (ui->maxPos->text().isEmpty()?max:(ui->maxPos->text().toDouble() >= max || ui->maxPos->text().toDouble() <= min)?max:ui->maxPos->text().toDouble());
    int steps = ui->num_steps->text().isEmpty()?500:ui->num_steps->text().toInt();
    this->step_size = (double)(this->stepper_max_limit-this->stepper_min_limit)/steps;//100 steps
    ui->size_steps->setText(QString::number(this->step_size));
    debug_out("Running a short scan from " + QString::number(this->stepper_min_limit) + " to " + QString::number(this->stepper_max_limit) + " with " + QString::number(steps) + " at the wavelength of " + " currently missing!", 1);
//    if(this->movementTime != 0)
//    {
//        this->homeStepper();
//        qDebug() << "Waiting till home in automat!";
//        QThread::sleep(3);
//        this->getEstimatedMovementTime(fabs(this->current_Stepper_Position-this->stepper_min_limit));
//        QThread::msleep(100);
//        this->moveStepperToAbsPosition(this->stepper_min_limit);
//        QThread::sleep((this->movementTime));
//    }
//    else
//    {
//        //this->homeStepper();
//        qDebug() << "Waiting till home!";
//        QThread::sleep(3);
//        this->moveStepperToAbsPosition(this->stepper_min_limit);
//        qDebug() << "Using stepsize of " << this->step_size;
//        qDebug() << "Please wait for movement stop and continue with the other button!";
//        this->getEstimatedMovementTime(this->step_size);
//        this->moveStepperToAbsPosition(this->stepper_min_limit);
//        QThread::sleep(3);
//        return;
//    }

//    this->getEstimatedMovementTime(this->step_size);
    emit this->moveStepperToAbsPosition(this->stepper_min_limit);
    ui->startScan->hide();
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
//        this->current_Stepper_Position = newPosition;
//    }
    emit this->AbsStepper(pos);
}

void MainWindow::moveStepperToRelPosition(double pos)
{
//    double newPosition = 0;
//    newPosition = pos;
//    if(this->current_Stepper_Position+newPosition <= this->stepper_max_limit && this->current_Stepper_Position + newPosition >= this->stepper_min_limit)
//    {
//        this->current_Stepper_Position = newPosition;
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
        this->curStep += this->step_size;
        emit this->getNewValue();
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
    this->dispValues.push_back(qMakePair(this->current_Stepper_Position, val));
    if(this->current_Stepper_Position + this->step_size < this->stepper_max_limit)
    {
        //this->moveStepperToAbsPosition(this->current_Stepper_Position + this->step_size);
        //qDebug() << "Stepper moving, step " << this->curStep << ", to position " << this->current_Stepper_Position << " with a stepsize of " << this->step_size << "!";
        debug_out("Stepper moving, step " + QString::number(this->curStep) + ", to position " + QString::number(this->current_Stepper_Position) + " with a stepsize of " + QString::number(this->step_size) + "!");
        this->curStep++;
        this->current_Stepper_Position += this->step_size;
        double progressValue = (double)(this->current_Stepper_Position - this->stepper_min_limit)/(double)(this->stepper_max_limit - this->stepper_min_limit);
        ui->scanProgress->setValue((int)(progressValue));
        emit this->moveStepperToAbsPosition(this->current_Stepper_Position);
        //this->getNewValue();
    }
    else
    {

        this->write_unformatted_file(this->dispValues, this->fileName);
        debug_out("Scan finished, moving back to first position!");
        //qDebug() << "Scan finished, moving back to first position!";
        this->moveStepperToAbsPosition(this->stepper_min_limit);
        if(this->multiAqu == false)
        {
            ui->minPos->setReadOnly(false);
            ui->maxPos->setReadOnly(false);
            ui->startScan->show();
            ui->minPos->setText("");
            ui->maxPos->setText("");
            ui->num_steps->setText("");
        }
        this->step_size = 0;
        this->scanRun = false;
        this->curStep = 0;
        this->replot();
    }
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
    this->curStep += this->step_size;
    emit this->getNewValue();
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
    this->wlSteps = 0;
}

void MainWindow::doFullScan()
{
    if(this->stepp == NULL || this->mono == NULL)
    {
        debug_out("One or both of the controllers are not connected, exiting!");
        return;
    }
    if(this->wlSteps + ui->startValue->text().toInt() < ui->stopValue->text().toInt())
    {
        debug_out("Scan " + QString::number(this->wlSteps) + " finished!");
        //qDebug() << "Scan " << this->wlSteps << " finished!";
        this->DispResults.push_back(qMakePair(this->current_Mono_Position, this->getMaxValue()));
        this->moveMonoToPosition(this->current_Mono_Position + this->wlSteps + 1);
        QThread::sleep(1);
        this->wlSteps++;
        this->fileName = QString::number(ui->startValue->text().toInt() + this->wlSteps);
        this->on_startScan_clicked();
    }
    else
    {
        this->multiAqu = false;
        this->write_unformatted_file(this->DispResults);
        this->wlSteps = 0;
        this->stepper_max_limit = MAX;
        this->stepper_min_limit = MIN;
        ui->startScan->show();
    }
}
