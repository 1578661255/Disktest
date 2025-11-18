#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include "opedb.h"
#include <QDir>
#include <QTimer>
class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    QString getName();
    void copyDir(QString strSrcDir, QString strDestDir);
    ~MyTcpSocket();
signals:
    void offline(MyTcpSocket *mysocket);


public slots:
    void recvMsg();
    void clientOffine();
    void sendFileToClient();


private:
    QString m_strName;

    QFile m_file;
    qint64 m_iTotal;
    qint64 m_iRecved;
    bool m_bUpload;

    QTimer *m_pTimer;

    // 新增：用于分块发送的缓冲区和文件对象
    char* m_pBuffer;          // 分块发送的缓冲区
    static const int BLOCK_SIZE = 4096; // 数据块大小
};

#endif // MYTCPSOCKET_H
