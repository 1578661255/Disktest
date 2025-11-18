#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "protocol.h"
#include <QTimer>
#include <QFile>
#include <QQueue>


class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFleList(const PDU *pdu);
      void clearEnterDir();
      QString getenterDir();
      void setDownloadFile(bool status);
       qint64 m_iTotal;
       qint64 m_iRecved;
    bool getDownloadStatus();
    QString getSaveFilePath();
    QString getShareFileName();

signals:
public slots:
    void createDir();
    void flushFile();
    void delDir();
    void updateFile();
//    void startUpload();
//    void uploadFileChunk();
//    void uploadNextChunk(qint64 bytesWritten = 0);
//    void uploadNextChunk();
    void renameFile();
    void enterDir(const QModelIndex &index);
    void returnPre();

    void uploadFileData();
    void delRegFile();

    void downloadFile();
    void shareFile();
     void moveFile();

     void selectDestDir();
private:


    QListWidget *m_pBookListW;
    QPushButton *m_pReturnPB;
    QPushButton *m_pCreateDirPB;
    QPushButton *m_pDelDirPB;
    QPushButton *m_pRenamePB;
    QPushButton *m_pFlushFilePB;
    QPushButton *m_pUploadPB;
    QPushButton *m_pDownLoadPB;
    QPushButton *m_pDelFilePB;
    QPushButton *m_pShareFilePB;
    QPushButton *m_pMoveFilePB;
    QPushButton *m_pSelectDirPB;
//    QPushButton *m_PB;
    QString m_strEnterDir;
    QString m_strUploadFilePath;

    QTimer *m_pTimer;
    QFile m_file;
    qint64 m_bytesSent = 0;
    qint64 m_totalBytes = 0;
//    QString m_strUploadFilePath;
    QString m_strSaveFilePath;
    bool m_bDownload;

    QString m_strShareFileName;

    QString m_strMoveFileName;
    QString m_strMoveFilePath;

    QString m_strDestDir;

    QQueue<QByteArray> m_chunkQueue;
    bool m_sending = false;

};



#endif // BOOK_H
