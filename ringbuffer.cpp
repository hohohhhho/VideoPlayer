#include "ringbuffer.h"

void RingBuffer::write(const QByteArray &data) {
    QMutexLocker lock(&mutex); // 自动加锁，离开作用域自动解锁

    // 计算可写入的第一段连续空间
    int writeSize = qMin(data.size(), buffer.size() - writePos);

    // 拷贝数据到缓冲区尾部
    memcpy(buffer.data() + writePos, data.constData(), writeSize);

    // 处理回绕写入（当数据跨越缓冲区末尾时）
    if (writeSize < data.size()) {
        int remaining = data.size() - writeSize;
        memcpy(buffer.data(), data.constData() + writeSize, remaining);
        writePos = remaining; // 更新写指针到缓冲区头部
    } else {
        writePos += writeSize; // 正常向前移动写指针
    }

    writePos %= buffer.size(); // 确保写指针在合法范围内
}

QByteArray RingBuffer::read(int maxSize) {
    QMutexLocker lock(&mutex);

    int avail = available(); // 获取当前可读数据量
    int readSize = qMin(maxSize, avail); // 实际读取量

    QByteArray result(readSize, 0); // 预分配结果内存

    // 第一段数据（从读指针到缓冲区末尾）
    int firstChunk = qMin(readSize, buffer.size() - readPos);
    memcpy(result.data(), buffer.data() + readPos, firstChunk);

    // 处理回绕读取
    if (firstChunk < readSize) {
        int secondChunk = readSize - firstChunk;
        memcpy(result.data() + firstChunk, buffer.data(), secondChunk);
        readPos = secondChunk; // 更新读指针到缓冲区头部
    } else {
        readPos += firstChunk; // 正常向前移动读指针
    }

    readPos %= buffer.size(); // 确保读指针合法
    return result;
}

int RingBuffer::available() const {
    // 计算有效数据长度（考虑回绕情况）
    return (writePos >= readPos) ?
               (writePos - readPos) :
               (buffer.size() - readPos + writePos);
}
