#include "filereader.h"
#include <QMetaObject>

FileReader::FileReader(const QString &filePath, QObject *parent)
    : QObject(parent), m_file(filePath)
{
}

void FileReader::start()
{
    if (!m_file.open(QIODevice::ReadOnly)) {
        emit error("无法打开文件: " + m_file.fileName());
        return;
    }
    QMetaObject::invokeMethod(this, "readNextChunk", Qt::QueuedConnection);
}

void FileReader::readNextChunk()
{
    if (!m_file.isOpen()) return;

    QByteArray buffer = m_file.read(CHUNK_SIZE);
    if (buffer.isEmpty()) {
        m_file.close();
        emit finished();
        return;
    }

    emit chunkReady(buffer);
    QMetaObject::invokeMethod(this, "readNextChunk", Qt::QueuedConnection);
}
