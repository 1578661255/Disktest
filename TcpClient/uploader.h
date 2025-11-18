#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QFile>

class FileUploader : public QObject {
    Q_OBJECT
public:
    FileUploader(const QString &filePath, QTcpSocket *socket, QObject *parent = nullptr);
    void start();

signals:
    void progress(qint64 sent, qint64 total);
    void finished();
    void error(QString err);

private slots:
    void sendNextChunk();

private:
    QFile m_file;
    QTcpSocket *m_socket;
    qint64 m_totalBytes = 0;
    qint64 m_bytesSent = 0;
};
