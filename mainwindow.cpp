#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
    this->monochromator = new serial_controller(list[ui->mono_Connections->currentIndex()].portName(), 1000, 9600, 1, false, false, false);
    connect(this->monochromator, &serial_controller::response, this, &MainWindow::Received_Mono_Data);
    connect(this, &MainWindow::executeCommandMono, this->monochromator, &serial_controller::transaction);
    QList<QString> initCommands;
    initCommands.append("Initialize Monochromator");
    initCommands.append("Goto position");
    initCommands.append("Get current position");
    ui->mono_command->addItems(initCommands);
    ui->stepper_command->setCurrentIndex(0);
    this->current_Mono_Command = "Initialize Monochromator";
}

void MainWindow::on_connect_stepper_clicked()
{
    QList<QSerialPortInfo> list;
    list = QSerialPortInfo::availablePorts();
    this->stepper = new serial_controller(list[ui->stepper_connections->currentIndex()].portName(), 1000, 57600, 1, false, false, false);
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
}

void MainWindow::on_Send_Data_Mono_clicked()
{
    if(this->monochromator != NULL)
    {
        if(this->current_Mono_Command == "Initialize Monochromator")
            emit this->executeCommandMono("\xFF\xFF\xFF", 0);
        else if(this->current_Mono_Command == "Goto position")
        {
            double value;
            if(ui->Mono_Value_1->text().isEmpty())
                value = 0;
            else
                value = ui->Mono_Value_1->text().toInt();

            emit this->executeCommandMono("10" + QString::number(value), 0);
        }
        else if(this->current_Mono_Command == "Get current position")
        {
            qDebug() << "\x38\x00";
            emit this->executeCommandMono("\x38\x00", 0);
        }
        else
            qDebug() << "Unknown mono command";
    }
}


void MainWindow::on_Send_Data_Stepper_clicked()
{
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
            emit this->executeCommandStepper("1PA" + QString::number(newPosition), 0);
        }
        else if(this->current_Stepper_Command == "Move relative")
        {
            double newPosition = 0;
            if(ui->Stepper_Value_1->text().isEmpty())
                newPosition = 0;
            else
                newPosition = ui->Stepper_Value_1->text().toDouble();
            emit this->executeCommandStepper("1PR" + QString::number(newPosition), 0);
        }
        else
            qDebug() << "Unknown command!";
    }
}


void MainWindow::Received_Mono_Data(QString &data)
{
    ui->mono_result->setText(data);
}

void MainWindow::Received_Stepper_Data(QString &data)
{
    ui->stepper_result->setText(data);
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

}

double MainWindow::getMaxValue()
{
    return 0;
}
