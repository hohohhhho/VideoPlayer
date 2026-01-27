#ifndef WHISPERTHREAD_H
#define WHISPERTHREAD_H

#include <QThread>
#include "ringbuffer.h"

class WhisperThread : public QThread
{
    Q_OBJECT
public:
    explicit WhisperThread(RingBuffer* buffer,QObject *parent = nullptr);

    void run()override;

signals:
    void newText(QString text);
private:
    RingBuffer* buffer;
};

#endif // WHISPERTHREAD_H
