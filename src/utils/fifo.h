#ifndef FIFO_H
#define FIFO_H

#include <QObject>
#include <QString>

class Fifo : public QObject
{
public:
    Fifo();
    ~Fifo();

    int Read(QString &data);
    int Write(QString data);
    int OpenRead();
    int OpenWrite();

private:
    int m_fd;
    QString m_fifoPath;
};

#endif //FIFO_H
