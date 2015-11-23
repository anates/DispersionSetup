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
    //load current serial port list
    QList<QSerialPortInfo> list;
    list = QSerialPortInfo::availablePorts();
    for(int i = 0; i < list.size(); i++)
    {
        ui->mono_Connections->addItem(list[i].portName());
        ui->stepper_connections->addItem(list[i].portName());
    }
    ui->scanProgress->setValue(0);
}

MainWindow::~MainWindow()
{
    if(this->monochromator != NULL)
        delete this->monochromator;
    if(this->stepper != NULL)
        delete this->stepper;
    delete ui;
}

void MainWindow::on_connectMono_clicked()
{
    QList<QSerialPortInfo> list;
    list = QSerialPortInfo::availablePorts();
    this->monochromator = new serial_controller(list[ui->mono_Connections->currentIndex()].portName(), 1000, 9600, 1, false, false, false, true);
    connect(this->monochromator, &serial_controller::response, this, &MainWindow::Received_Mono_Data);
    connect(this, &MainWindow::executeCommandMono, this->monochromator, &serial_controller::transaction);
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
    this->stepper = new serial_controller(list[ui->stepper_connections->currentIndex()].portName(), 1000, 57600, 1, false, false, false, false);
    connect(this->stepper, &serial_controller::response, this, &MainWindow::Received_Stepper_Data);
    connect(this, &MainWindow::executeCommandStepper, this->stepper, &serial_controller::transaction);
    QList<QString> initCommand;
    initCommand.append("Initialize stepper motor controller");
    initCommand.append("Home controller");
    initCommand.append("Get current position");
    initCommand.append("Move absolute");
    initCommand.append("Move relative");
    ui->stepper_command->addItems(initCommand);
    ui->stepper_command->setCurrentIndex(0);
    this->current_Stepper_Command = "Initialize stepper motor controller";
    ui->connect_stepper->hide();
}

void MainWindow::on_Send_Data_Mono_clicked()
{
    this->mono_response.clear();
    ui->mono_result->setText("");
    if(this->monochromator != NULL)
    {
        if(this->current_Mono_Command == "Initialize Monochromator")
            emit this->executeCommandMono("FFFFFF", 0);
        else if(this->current_Mono_Command == "Goto position")
        {
            double value;
            if(ui->Mono_Value_1->text().isEmpty())
                value = 0;
            else
                value = ui->Mono_Value_1->text().toInt();
            QString wavelength;
            if(QString::number((qlonglong)value, 16).length() == 3)
                wavelength = "0" + QString::number((qlonglong)value, 16);
            else
                wavelength = QString::number((qlonglong)value, 16);
            emit this->executeCommandMono("10" + wavelength, 0);
        }
        else if(this->current_Mono_Command == "Get current position")
        {
            qDebug() << "3800";
            emit this->executeCommandMono("3800", 0);
        }
        else
            qDebug() << "Unknown mono command";
    }
    ui->Mono_Value_1->setText("");
}


void MainWindow::on_Send_Data_Stepper_clicked()
{
    this->stepper_response.clear();
    ui->stepper_result->setText("");
    if(this->stepper != NULL)
    {
        if(this->current_Stepper_Command == "Initialize stepper motor controller")
        {
            emit this->executeCommandStepper("1OR?", 0.01);
        }
        else if(this->current_Stepper_Command == "Home controller")
        {
            emit this->executeCommandStepper("1OR?", 0);
        }
        else if(this->current_Stepper_Command == "Get current position")
        {
            emit this->executeCommandStepper("1TP?", 0);
        }
        else if(this->current_Stepper_Command == "Move absolute")
        {
            double newPosition = 0;
            if(ui->Stepper_Value_1->text().isEmpty())
                newPosition = 0;
            else
                newPosition = ui->Stepper_Value_1->text().toDouble();
            if((newPosition >= this->stepper_min_limit) && (newPosition <= this->stepper_max_limit))
            {
                emit this->executeCommandStepper("1PA" + QString::number(newPosition), 0);
                this->current_Stepper_Position = newPosition;
            }
        }
        else if(this->current_Stepper_Command == "Move relative")
        {
            double newPosition = 0;
            if(ui->Stepper_Value_1->text().isEmpty())
                newPosition = 0;
            else
                newPosition = ui->Stepper_Value_1->text().toDouble();
            if(this->current_Stepper_Position+newPosition <= this->stepper_max_limit && this->current_Stepper_Position + newPosition >= this->stepper_min_limit)
            {
                this->current_Stepper_Position = newPosition;
                emit this->executeCommandStepper("1PR" + QString::number(newPosition), 0);
            }
        }
        else
            qDebug() << "Unknown command!";
    }
    ui->Stepper_Value_1->setText("");
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
        qDebug() << data_int;
    }
    if(data_int.last() == 24 && data_int[data_int.length()-2] == 17)
        qDebug() << "Everything ok";
    if(data_int.size() >= 4)
    {
        WL = 0;
        QVector<int> values;
        for(int i = 0; i < data_int.size()-1; i++)
            if(data_int[i+1] != 24)//Can lead to a buffer overflow, better solution?
                values.push_back((data_int[i] > 0)?data_int[i]:128+data_int[i]);
        for(int i = 0; i < values.size(); i++)
        {
            qDebug() << "I: " << i << " & value: " << values[i];
            qDebug() << "Current value is: " << QString::number((values[i] < 15)?values[i]*pow(16, values.size() - (i+1)):(values[i] < 65)?(values[i] - 32)*pow(16, values.size() - (i+1)):(values[i]-56)*pow(16, values.size() - (i+1))) << " at " << i;
            WL += (values[i] < 15)?values[i]*pow(16, values.size() - (i+1)):(values[i] < 65)?(values[i] - 32)*pow(16, values.size() - (i+1)):(values[i]-56)*pow(16, values.size() - (i+1));
        }
    }
    if(WL != -1)
        ui->mono_result->setText(QString::number(WL));
}

void MainWindow::Received_Stepper_Data(QString &data)
{
    this->stepper_response += data;
    ui->stepper_result->setText(this->stepper_response);
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
    ui->resultPlot->setAxisTitle(QwtPlot::xBottom, "Wavelength [nm]");
    QVector<double> x;
    QVector<double> y;
    if(this->dispValues.isEmpty() == false)
    {
        vectorToMap(this->dispValues, this->plotData);
        x = QVector<double>::fromList(this->plotData.keys());
        y = QVector<double>::fromList(this->plotData.values());
        this->Curve.setSamples(x, y);
        this->Curve.attach(ui->resultPlot);
        ui->resultPlot->updateAxes();
        ui->resultPlot->show();
        ui->resultPlot->replot();
    }
}

void MainWindow::on_startScan_clicked()
{
    if(ui->startValue->text().isEmpty() || ui->stopValue->text().isEmpty() || ui->startValue->text().toInt() >= ui->stopValue->text().toInt())
        return;

    for(int i = 0; i < ui->stopValue->text().toInt() - ui->startValue->text().toInt(); i++)
    {
        this->dispValues.push_back(qMakePair(i + ui->startValue->text().toInt(), this->getMaxValue()));
    }
    this->replot();

}

double MainWindow::getMaxValue()
{
    return 0;
}
