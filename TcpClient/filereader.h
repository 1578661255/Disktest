#ifndef FILEREADER_H
#define FILEREADER_H

#include <QObject>
#include <QFile>
#include <QByteArray>

class FileReader : public QObject
{
    Q_OBJECT
public:
    explicit FileReader(const QString &filePath, QObject *parent = nullptr);
    void start();   // 开始读取

signals:
    void chunkReady(QByteArray data);  // 文件块准备好
    void finished();                   // 文件读取完成
    void error(QString err);

private slots:
    void readNextChunk();

private:
    QFile m_file;
    const qint64 CHUNK_SIZE = 64*1024; // 每块64KB
};

#endif // FILEREADER_H
