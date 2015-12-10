#ifndef DATA_CLEANER_H
#define DATA_CLEANER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QThread>
#include <QTime>
#include <QWaitCondition>
#include <QMutex>
#include <QRegExp>
#include <QDebug>

#define MAX_LENGTH 1000

struct buffer_ring{
    const int DataSize = 1;

    const int BufferSize = 28;
    QVector<QString> buffer;
    int lastPos = 0;
    QWaitCondition bufferNotEmpty;
    QWaitCondition bufferNotFull;
    QMutex mutex;
    int numUsedBytes = 0;
};

class data_filler : public QThread
{
    Q_OBJECT
private:
    buffer_ring *buffer;
public:
    data_filler(buffer_ring *buffer, QObject *parent = 0) : QThread(parent)
    {
        this->buffer = buffer;
    }

    void run() Q_DECL_OVERRIDE
    {
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

        for (int i = 0; i < this->buffer->DataSize; ++i) {
            this->buffer->mutex.lock();
            if (this->buffer->numUsedBytes == this->buffer->BufferSize)
                this->buffer->bufferNotFull.wait(&(this->buffer->mutex));
            this->buffer->mutex.unlock();

            this->buffer->buffer[i % this->buffer->BufferSize] = "ACGT"[(int)qrand() % 4];

            this->buffer->mutex.lock();
            this->buffer->numUsedBytes += 1;
            this->buffer->bufferNotEmpty.wakeAll();
            this->buffer->mutex.unlock();
        }
    }
signals:
    void full_data(QString);
public slots:
    //void check(void);
    //void fill(QString data);

};

class data_writer : public QThread
{
    Q_OBJECT
private:
    buffer_ring *buffer;
    QString data;
    QRegExp filter;
public:
    explicit data_writer(buffer_ring *buffer, QObject * parent = 0) : QThread(parent)
    {
        this->buffer = buffer;
        this->filter = QRegExp("[A-z][A-z]\\d{1,2}.\\d+");
    }

    void run() Q_DECL_OVERRIDE
    {
        for (int i = 0; i < this->buffer->DataSize; ++i) {
            this->buffer->mutex.lock();
            if (this->buffer->numUsedBytes == 0)
                this->buffer->bufferNotEmpty.wait(&(this->buffer->mutex));
            this->buffer->mutex.unlock();

            //fprintf(stderr, "%c", this->buffer->buffer[i % this->buffer->BufferSize]);

            this->buffer->mutex.lock();
            this->buffer->numUsedBytes--;
            this->buffer->bufferNotFull.wakeAll();
            this->buffer->mutex.unlock();
        }
        //fprintf(stderr, "\n");
    }
signals:
public slots:
};

class data_worker : public QObject
{
    Q_OBJECT
private:
    buffer_ring *buffer;
    QVector<QString> data;
    QRegExp filter;
public:
    explicit data_worker(buffer_ring *buffer, QObject *parent = 0);
    ~data_worker();
signals:
    void cleanedData(QString);
public slots:
    void startCleaning(void);

};

class data_cleaner : public QObject
{
    Q_OBJECT
private:
    data_filler *filler = NULL;
    data_writer *writer = NULL;
    buffer_ring buffer;
    QThread workerThread;
public:
    explicit data_cleaner(QObject *parent = 0);
    ~data_cleaner();
signals:
    void Data(void);
    void overFlow(void);
    void resultData(QString);
public slots:
    void newData(QString UncData);
    void newCleanData(QString data);

};

#endif // DATA_CLEANER_H
