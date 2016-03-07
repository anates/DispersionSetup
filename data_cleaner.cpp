#include "data_cleaner.h"
//Data_worker
data_worker::data_worker(buffer_ring *buffer, QObject *parent)
{
    UNUSED(parent);
    this->buffer = buffer;
    this->filter = QRegExp("[A-z][A-z]\\d{1,2}.\\d+");
}

data_worker::~data_worker()
{

}

void data_worker::startCleaning()
{
    this->buffer->mutex.lock();
    if(this->buffer->buffer.size() < 5)
        return;
    else
    {
        int i = 0;
        QRegExp re("\\d{1}");
        while(this->buffer->buffer.size() > 0 && re.exactMatch(QString(this->buffer->buffer.first())) == false)
        {
            this->buffer->buffer.pop_front();
            this->buffer->numUsedBytes--;
            this->buffer->lastPos--;
            //qDebug() << "Buffer size: " << this->buffer->buffer.size();
            debug_out("Buffer size: " + QString::number(this->buffer->buffer.size()));

        }
        if(this->buffer->buffer.size() == 0)
        {
            this->buffer->mutex.unlock();
            return;
        }
        i = 0;
        if(re.exactMatch(this->buffer->buffer.first()))
        {
            while(i < this->buffer->buffer.size())
            {
                this->data.append(QString(this->buffer->buffer[i]));

                this->buffer->numUsedBytes--;
                this->buffer->lastPos--;
                i++;
            }
        }
        this->buffer->mutex.unlock();
        if(this->data.contains("\x00D") || this->data.contains("\x00A"))
        {
            QString return_string;
            for(int j = 0; j < data.size(); j++)
                if(this->data[j] != "\x00D" && this->data[j] != "\x00A")
                    return_string += data[j];
                else
                    break;
            if(QRegExp("\\d{1}[A-z][A-z]\\d{1,2}.\\d+").exactMatch(return_string))
                emit this->cleanedData(return_string);
            this->data.clear();
            return_string = "";
        }
        else
            this->data.clear();
    }

}

//Data_cleaner

data_cleaner::data_cleaner(QObject *parent) : QObject(parent)
{
    this->buffer.numUsedBytes = 0;
    this->buffer.buffer.reserve(30);
    this->buffer.lastPos = 0;
    data_worker *worker = new data_worker(&(this->buffer));
    worker->moveToThread(&(this->workerThread));
    connect(this, &data_cleaner::Data, worker, &data_worker::startCleaning);
    connect(worker, &data_worker::cleanedData, this, &data_cleaner::newCleanData);
    connect(&(this->workerThread), &QThread::finished, worker, &data_worker::deleteLater);
    this->workerThread.start();
    //this->filler = new data_filler(&(this->buffer));
    //this->writer = new data_writer(&(this->buffer));
    //this->filler->start();
    //this->writer->start();
}

data_cleaner::~data_cleaner()
{
    if(this->writer != NULL)
    {
        this->writer->wait();
        delete this->writer;
    }
    if(this->filler != NULL)
    {
        this->filler->wait();
        delete this->filler;
    }
    this->workerThread.quit();
    this->workerThread.wait();

}

void data_cleaner::newData(QString UncData)
{
    if(this->buffer.buffer.size() > MAX_LENGTH)
    {
        emit this->overFlow();
        return;
    }
    this->buffer.mutex.lock();
    int j = 0;
    for(int i = this->buffer.lastPos; i < UncData.size(); i++)
    {
        if(i < this->buffer.buffer.size())
            this->buffer.buffer[i] = QString(UncData[j]);
        else
            this->buffer.buffer.append(QString(UncData[j]));
        j++;
    };
    this->buffer.lastPos += UncData.size();
    this->buffer.numUsedBytes += UncData.size();
    this->buffer.mutex.unlock();
    emit this->Data();

}

void data_cleaner::newCleanData(QString data)
{
    emit this->resultData(data);
}
