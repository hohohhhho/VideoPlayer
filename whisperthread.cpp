#include "whisperthread.h"
#include <whisper.h>
#include <QDebug>

WhisperThread::WhisperThread(RingBuffer *buffer, QObject *parent)
    : QThread{parent},buffer(buffer)
{}

void WhisperThread::run()
{
    // === 1. 初始化模型参数 ===
    struct whisper_context_params cparams /*= {
        .use_gpu = false  // 仅使用CPU
    }*/;

    // === 2. 加载模型 ===
    struct whisper_context *ctx = whisper_init_from_file_with_params("ggml-base.bin", cparams);
    if (!ctx) {
        qCritical() << "模型加载失败";
        return;
    }

    // === 3. 配置推理参数 ===
    struct whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.language = "zh";
    params.n_threads = 4;
    params.single_segment = true;

    // === 4. 主处理循环 ===
    while (!isInterruptionRequested()) {
        QByteArray data = buffer->read(16000 * 2); // 读取1秒数据
        if (data.isEmpty()) {
            QThread::msleep(10);
            continue;
        }

        // 转换为float32格式
        const int16_t *pcm = reinterpret_cast<const int16_t*>(data.constData());
        std::vector<float> pcmf32(data.size() / sizeof(int16_t));
        for (size_t i=0; i<pcmf32.size(); i++) {
            pcmf32[i] = pcm[i] / 32768.0f;
        }

        // 执行识别
        if (whisper_full(ctx, params, pcmf32.data(), pcmf32.size()) != 0) {
            qWarning() << "识别失败";
            continue;
        }

        // 获取结果
        QString text;
        for (int i=0; i<whisper_full_n_segments(ctx); i++) {
            text += QString::fromUtf8(whisper_full_get_segment_text(ctx, i));
        }
        emit newText(text.trimmed());
    }

    // === 5. 释放资源 ===
    whisper_free(ctx);

    // // 主循环：直到线程被请求中断
    // while (!isInterruptionRequested()) {
    //     // 从环形缓冲区读取2秒音频（16000样本/秒 × 2秒 × 2字节/样本 = 64000字节）
    //     QByteArray data = buffer->read(64000);

    //     if (data.isEmpty()) {
    //         QThread::msleep(10); // 避免忙等待，减少CPU占用
    //         continue;
    //     }

    //     // 转换为Whisper所需的float32格式（-1.0~1.0范围）
    //     const int16_t *pcm = reinterpret_cast<const int16_t*>(data.constData());
    //     std::vector<float> pcmf32(data.size() / sizeof(int16_t));
    //     for (size_t i = 0; i < pcmf32.size(); i++) {
    //         pcmf32[i] = pcm[i] / 32768.0f; // 16-bit有符号整数归一化
    //     }

    //     // 配置识别参数
    //     whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    //     params.language = "zh";        // 设置识别语言为中文
    //     params.n_threads = 4;          // 使用4个CPU线程加速
    //     params.single_segment = true;  // 将整个音频视为单一段落（适合实时流）

    //     // 执行语音识别
    //     if (whisper_full(ctx, params, pcmf32.data(), pcmf32.size()) == 0) {
    //         QString text;
    //         // 遍历所有识别段落
    //         for (int i = 0; i < whisper_full_n_segments(ctx); i++) {
    //             // 获取段落文本并拼接
    //             text += QString::fromStdString(whisper_full_get_segment_text(ctx, i));
    //         }
    //         emit newText(text.trimmed()); // 发送信号更新UI
    //     } else {
    //         qDebug() << "Whisper识别失败!";
    //     }
    // }

    // // 清理资源
    // whisper_free(ctx);
}
