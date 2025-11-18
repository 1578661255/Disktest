#include "uploader.h"
#include <QTimer>

FileUploader::FileUploader(const QString &filePath, QTcpSocket *socket, QObject *parent)
    : QObject(parent), m_socket(socket)
{
    m_file.setFileName(filePath);
}

void FileUploader::start()
{
    if (!m_file.open(QIODevice::ReadOnly)) {
        emit error("无法打开文件");
        return;
    }
    m_totalBytes = m_file.size();
    m_bytesSent = 0;

    QTimer::singleShot(0, this, &FileUploader::sendNextChunk);
}

void FileUploader::sendNextChunk()
{
    if (!m_file.isOpen()) return;

    const qint64 chunkSize = 4096;
    QByteArray data = m_file.read(chunkSize);
    if (data.isEmpty()) {
        emit finished();
        m_file.close();
        return;
    }

    qint64 bytesWritten = m_socket->write(data);
    if (bytesWritten == -1) {
        emit error("发送失败");
        m_file.close();
        return;
    }

    m_bytesSent += bytesWritten;
    emit progress(m_bytesSent, m_totalBytes);

    // 等待缓冲区可写时再继续
    connect(m_socket, &QTcpSocket::bytesWritten, this, [this](qint64) {
        QTimer::singleShot(0, this, &FileUploader::sendNextChunk);
    });
}
