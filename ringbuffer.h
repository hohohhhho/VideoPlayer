#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <QMutex>
#include <QObject>

class RingBuffer
{
public:
    // 构造函数：初始化缓冲区大小（默认容纳5秒16kHz 16-bit单声道音频）
    explicit RingBuffer(int size = 16000 * 2 * 5) {
        buffer.resize(size); // QByteArray内部使用连续内存，适合快速拷贝
    }

    // 写入音频数据到缓冲区（线程安全）
    void write(const QByteArray &data);

    // 从缓冲区读取指定大小的数据（线程安全）
    QByteArray read(int maxSize);

private:
    QByteArray buffer;  // 存储音频数据的底层数组
    int readPos = 0;    // 当前读取位置（字节偏移）
    int writePos = 0;   // 当前写入位置（字节偏移）
    QMutex mutex;       // 互斥锁，保护共享数据访问

    // 计算当前可用数据量（私有方法，无需线程安全）
    int available() const;

signals:
};

#endif // RINGBUFFER_H
