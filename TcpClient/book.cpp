#include "book.h"
#include "filereader.h"
#include "uploader.h"
//#include <pthread.h>
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QThread>
#include "opewidget.h"
#include <QFileDialog>
#include "sharefile.h"

Book::Book(QWidget *parent) : QWidget(parent)
{
    m_strEnterDir.clear();

    m_bDownload=false;
    m_pTimer=new QTimer;

     m_pBookListW=new QListWidget;
     m_pReturnPB =new QPushButton("返回");
     m_pCreateDirPB=new QPushButton("创建文件夹");
     m_pDelDirPB =new QPushButton("删除文件夹");
     m_pRenamePB =new QPushButton("重命名");
     m_pFlushFilePB =new QPushButton("刷新文件");
    m_pMoveFilePB =new QPushButton("移动文件");
    m_pSelectDirPB =new QPushButton("目标目录");
    m_pSelectDirPB->setEnabled(false);

     QVBoxLayout *pDirVBL =new QVBoxLayout;
     pDirVBL->addWidget(m_pReturnPB);
     pDirVBL->addWidget(m_pCreateDirPB);
     pDirVBL->addWidget(m_pDelDirPB);
     pDirVBL->addWidget(m_pRenamePB);
     pDirVBL->addWidget(m_pFlushFilePB);
     pDirVBL->addWidget(m_pSelectDirPB);

//     pDirVBL->addWidget(m_pReturnPB);


     m_pUploadPB =new QPushButton("上传文件");
     m_pDownLoadPB =new QPushButton("下载");
     m_pDelFilePB =new QPushButton("删除文件");
     m_pShareFilePB =new QPushButton("共享文件");

     QVBoxLayout *pFileVBL =new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDownLoadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    QHBoxLayout *pMain =new QHBoxLayout;
    pMain->addWidget(m_pBookListW);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);

    connect(m_pCreateDirPB,SIGNAL(clicked(bool)),
            this,SLOT(createDir()));
    connect(m_pFlushFilePB,SIGNAL(clicked(bool)),
            this,SLOT(flushFile()));
    connect(m_pDelDirPB,SIGNAL(clicked(bool)),
            this,SLOT(delDir()));
    connect(m_pRenamePB,SIGNAL(clicked(bool)),
            this,SLOT(renameFile()));
//    connect(m_pBookListW,SIGNAL(doubleClicked(QModelIndex)),
//            this,SLOT(enterDir(QModelIndex)));
//    connect(m_pBookListW, &QListWidget::doubleClicked,
//            this, &MyClass::enterDir);
    connect(m_pBookListW,&QListWidget::doubleClicked,
            this,&Book::enterDir);
    connect(m_pReturnPB,&QPushButton::clicked,
            this,&Book::returnPre);
    connect(m_pUploadPB,&QPushButton::clicked,
            this,&Book::updateFile);
    connect(m_pTimer,&QTimer::timeout,
            this,&Book::uploadFileData);
    connect(m_pDelFilePB,&QPushButton::clicked,
            this,&Book::delRegFile);
    connect(m_pDownLoadPB,&QPushButton::clicked,
            this,&Book::downloadFile);
    connect(m_pShareFilePB,&QPushButton::clicked,
            this,&Book::shareFile);
    connect(m_pMoveFilePB,&QPushButton::clicked,
            this,&Book::moveFile);
    connect(m_pSelectDirPB,&QPushButton::clicked,
            this,&Book::selectDestDir);


}

void Book::updateFleList(const PDU *pdu)
{
       if (NULL ==pdu) {
           return ;
       }
       QListWidgetItem *pItemTmp=nullptr;
       int row =m_pBookListW->count();
       while (m_pBookListW->count()>0) {
           pItemTmp = m_pBookListW->item(row-1);
           m_pBookListW->removeItemWidget(pItemTmp);
           delete pItemTmp;
           row=row-1;
       }

       FileInfo *pFileInfo=nullptr;
       int iCount =pdu->uiMsgLen/sizeof(FileInfo);
       for (int i=0;i<iCount;i++) {
            pFileInfo =(FileInfo*)(pdu->caMsg)+i;
            qDebug()<<pFileInfo->caName<<pFileInfo->iFileType;
            QListWidgetItem *pItem= new QListWidgetItem;
            if (0 == pFileInfo->iFileType) {
                 pItem->setIcon(QIcon(QPixmap(":/map/dir.jpeg")));
            }else {
                pItem->setIcon(QIcon(QPixmap(":/map/reg.jpeg")));
            }
            pItem->setText(pFileInfo->caName);
            m_pBookListW->addItem(pItem);
       }
}

void Book::clearEnterDir()
{
    m_strEnterDir.clear();
}

QString Book::getenterDir()
{
    return m_strEnterDir;
}

void Book::setDownloadFile(bool status)
{
    m_bDownload  = status;
}

bool Book::getDownloadStatus()
{
    return m_bDownload;
}

QString Book::getSaveFilePath()
{
    return m_strSaveFilePath;
}

QString Book::getShareFileName()
{
    return m_strShareFileName;
}

void Book::moveFile()
{
   QListWidgetItem *pCurItem =m_pBookListW->currentItem();
    if (NULL != pCurItem) {
        m_strMoveFileName = pCurItem->text();
        QString strCurPath =TcpClient::getInstance().curPath();
        m_strMoveFilePath= strCurPath+'/'+m_strShareFileName;

        m_pSelectDirPB->setEnabled(true);

    }else {

        QMessageBox::warning(this,"移动文件","请选择文件");

    }
}

void Book::selectDestDir()
{
    qDebug()<<"====IN SELECT DIR";
     QListWidgetItem *pCurItem =m_pBookListW->currentItem();
     if (NULL != pCurItem) {
         QString strDestDir = pCurItem->text();
         QString strCurPath =TcpClient::getInstance().curPath();
         m_strDestDir= strCurPath+'/'+strDestDir;

         int srcLen =m_strMoveFilePath.size();
         int destLen =m_strDestDir.size();

         PDU *pdu =mkPDU(srcLen+destLen+2);
         pdu->uiMsgType=ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
         sprintf(pdu->caData,"%d %d %s",srcLen,destLen,m_strMoveFileName.toStdString().c_str());

         memcpy(pdu->caMsg,m_strMoveFilePath.toStdString().c_str(),srcLen);
         memcpy((char*)(pdu->caMsg)+(srcLen+1),m_strDestDir.toStdString().c_str(),destLen);

        qDebug()<<"m_strMoveFilePath::"<<m_strMoveFilePath<<"m_strDestDir::"<<m_strDestDir;

        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;


     }else {

         QMessageBox::warning(this,"移动文件","请选择文件");

     }
     m_pSelectDirPB->setEnabled(false);
}



void Book::createDir()
{
    QString strNewDir=QInputDialog::getText(this,"新建文件夹","新文件夹名字");
    if (!strNewDir.isEmpty()) {
        if (strNewDir.size()>32) {
            QMessageBox::warning(this,"新建文件夹","c长度超过32");
        }else {
            QString strName=TcpClient::getInstance().loginName();
            QString strCurPath =TcpClient::getInstance().curPath();

            PDU *pdu =mkPDU(strCurPath.size()+1);
            pdu->uiMsgType= ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            strncpy(pdu->caData,strName.toStdString().c_str(),strName.size());
            strncpy(pdu->caData+32,strNewDir.toStdString().c_str(),strNewDir.size());
            memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());

            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
            free(pdu);
            pdu=nullptr;
        }
    }else {
        QMessageBox::warning(this,"新建文件夹","文件夹名不能为空");
    }
}

void Book::flushFile()
{
    QString strCurPath=TcpClient::getInstance().curPath();
    PDU *pdu=mkPDU(strCurPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strncpy((char*)(pdu->caMsg),strCurPath.toStdString().c_str(),strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=nullptr;
}

void Book::delDir()
{
    QString strCurPath=TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (NULL == pItem) {
        QMessageBox::warning(this,"删除文件","请选择删除的文件。");
    }else {
        QString strDelName = pItem->text();
        PDU *pdu =mkPDU(strCurPath.size()+1);
        pdu->uiMsgType=ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        strncpy(pdu->caData,strDelName.toStdString().c_str(),strDelName.size());
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;
    }
}

void Book::updateFile()
{
    QString strCurName= TcpClient::getInstance().curPath();
    m_strUploadFilePath=QFileDialog::getOpenFileName();
    if (m_strUploadFilePath.isEmpty()) {
        QMessageBox::warning(this,"上传文件","文件名不能为空");

    }else {
        int index=m_strUploadFilePath.lastIndexOf('/');
        QString strFileName= m_strUploadFilePath.right(m_strUploadFilePath.size()-index-1);
        qDebug()<<strFileName;

        QFile file(m_strUploadFilePath);
        qint64 fileSize=file.size();

        QString strCurPath=TcpClient::getInstance().curPath();
        PDU *pdu =mkPDU(strCurPath.size()+1);
        pdu->uiMsgType =ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        sprintf(pdu->caData,"%s %lld",strFileName.toStdString().c_str(),fileSize);

        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;

        m_pTimer->start(1000);
    }

}



void Book::renameFile()
{
     QString strCurPath=TcpClient::getInstance().curPath();
     QListWidgetItem *pItem = m_pBookListW->currentItem();
     if (NULL == pItem) {
         QMessageBox::warning(this,"重名文件","请选择要重命名的文件。");
     }else {
        QString strOldName=pItem->text();
        QString strNewName=QInputDialog::getText(this,"重命名文件","输入新文件名");
        if (strNewName.isEmpty()) {
            QMessageBox::warning(this,"重命名文件","新文件名");

        }else {
            PDU *pdu =mkPDU(strCurPath.size()+1);
            pdu->uiMsgType=ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            strncpy(pdu->caData,strOldName.toStdString().c_str(),strOldName.size());
            strncpy(pdu->caData+32,strNewName.toStdString().c_str(),strNewName.size());
            memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());

            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
            free(pdu);
            pdu=nullptr;
        }
     }
}

void Book::enterDir(const QModelIndex &index)
{
    qDebug()<<"=========IN Book::enterDir";
    QString strDirName = index.data().toString();
    qDebug()<< strDirName;
    m_strEnterDir=strDirName;
    QString strCurPath =TcpClient::getInstance().curPath();
    PDU *pdu =mkPDU(strCurPath.size()+1);
    pdu->uiMsgType=ENUM_MSG_TYPE_ENTER_FILE_REQUEST;
    strncpy(pdu->caData,strDirName.toStdString().c_str(),strDirName.size());
    memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());

    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=nullptr;


}

void Book::returnPre()
{
    qDebug()<<"=====IN:RETURN_PRE";
    QString strCurPath =TcpClient::getInstance().curPath();
    QString strRootPath ="./"+TcpClient::getInstance().loginName();
    if (strCurPath ==strRootPath) {
        QMessageBox::warning(this,"返回","返回失败，已经在根目录");
    }else {
        int index = strCurPath.lastIndexOf('/');
        strCurPath.remove(index,strCurPath.size()-index);
        TcpClient::getInstance().setCurPath(strCurPath);

        flushFile();
    }
}
//void Book::uploadFileData()
//{
//    qDebug()<<"in timer!!!\n";
//    m_pTimer->stop();
//    QFile file(m_strUploadFilePath);
//    if (!file.open(QIODevice::ReadOnly)) {
//        QMessageBox::warning(this, "上传文件", "打开文件失败");
//        return;
//    }

//    QTcpSocket &socket = TcpClient::getInstance().getTcpSocket();
//    QByteArray buffer;
//    buffer.resize(4096);

//    qint64 ret = 0;
//    while ((ret = file.read(buffer.data(), buffer.size())) > 0) {
//        qint64 bytesWritten = 0;
//        while (bytesWritten < ret) {
//            qint64 n = socket.write(buffer.constData() + bytesWritten, ret - bytesWritten);
//            if (n == -1) {
//                QMessageBox::warning(this, "上传文件", "网络发送失败");
//                file.close();
//                return;
//            }
//            bytesWritten += n;
//            // 等待缓冲区腾出空间
//            if (!socket.waitForBytesWritten(3000)) {
//                QMessageBox::warning(this, "上传文件", "发送超时");
//                file.close();
//                return;
//            }
//        }
//    }

//    file.close();
//    qDebug() << "文件上传完成";
//}

void Book::uploadFileData()
{
    m_pTimer->stop();
    qDebug()<<"in Timer::upload";
    QFile file(m_strUploadFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this,"上传文件","打开文件失败");
        return ;
    }

    char *pBuffer =new char[4096];
    qint64 ret=0;
    while (true) {
        ret=file.read(pBuffer,4096);
        if (ret>0 && ret<=4096) {
            TcpClient::getInstance().getTcpSocket().write(pBuffer, ret);
        }else if (0 == ret) {
            break;
        }else {
            QMessageBox::warning(this,"上传文件","上传失败，读文件失败");
            break;
        }
    }
    file.close();
    delete [] pBuffer;
    pBuffer=nullptr;

}

void Book::delRegFile()
{
    QString strCurPath=TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (NULL == pItem) {
        QMessageBox::warning(this,"删除文件","请选择删除的文件。");
    }else {
        QString strDelName = pItem->text();
        PDU *pdu =mkPDU(strCurPath.size()+1);
        pdu->uiMsgType=ENUM_MSG_TYPE_DEL_FILE_REQUEST;
        strncpy(pdu->caData,strDelName.toStdString().c_str(),strDelName.size());
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;
    }
}

void Book::downloadFile()
{
    qDebug()<<"IN::BOOK::downFIle";
    QString strCurPath=TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (NULL == pItem) {
        QMessageBox::warning(this,"下载文件","请选择下载的文件。");
    }else {
        QString strSaveFilePath = QFileDialog::getSaveFileName();
        if (strSaveFilePath.isEmpty()) {
            QMessageBox::warning(this,"下载问价","请指定要保存的位置");
            m_strSaveFilePath.clear();
        }else {
            m_strSaveFilePath=strSaveFilePath;
//            m_bDownload=true;
        }


        QString strCurPath =TcpClient::getInstance().curPath();
        PDU *pdu=mkPDU(strCurPath.size()+1);
        pdu->uiMsgType=ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
        QString strFileName= pItem->text();
        strcpy(pdu->caData,strFileName.toStdString().c_str());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(),strCurPath.size());

        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);


    }
}

void Book::shareFile()
{
    qDebug()<<"IN::BOOK::downFIle";
    QString strCurPath=TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (NULL == pItem) {
        QMessageBox::warning(this,"共享文件","请选择共享的文件。");
        return ;
    }else {
       m_strShareFileName =pItem->text();

    }
    qDebug()<<"+++++IN SHARE++";
    Friend *pFriend =OpeWidget::getInstance().getFriend();
    QListWidget *pFriendList =pFriend->getFriendList();
    qDebug()<<"COUNT::"<<pFriendList->count();
    ShareFile::getInstance().updateFriend(pFriendList);

    for (int i=0;i<pFriendList->count();i++) {
        qDebug()<<pFriendList->item(i)->text();
    }
    if (ShareFile::getInstance().isHidden()) {
        ShareFile::getInstance().show();
    }
    qDebug()<<"+++++OUT SHARE++";

}
