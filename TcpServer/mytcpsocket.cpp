#include "mytcpsocket.h"
#include <QDebug>
#include "mytcpserver.h"
#include <QDir>
#include <QFileInfoList>
#include <QFile>
MyTcpSocket::MyTcpSocket()
{
    connect(this, SIGNAL(readyRead())
            ,this, SLOT(recvMsg()));
    connect(this, SIGNAL(disconnected()),
            this,SLOT(clientOffine()));


    m_bUpload=false;
    m_pTimer = new QTimer;
    connect(m_pTimer,&QTimer::timeout ,
            this,&MyTcpSocket::sendFileToClient);

    // 初始化缓冲区指针
    m_pBuffer = nullptr;
}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::copyDir(QString strSrcDir, QString strDestDir)
{
    QDir dir;
    dir.mkdir(strDestDir);

    dir.setPath(strSrcDir);
    QFileInfoList fileInfoList = dir.entryInfoList();

    QString srcTmp;
    QString destTmp;

    for (int i=0;i<fileInfoList.size();i++) {
        if (fileInfoList[i].isFile()) {
            qDebug()<<"filename:"<<fileInfoList[i].fileName();
            srcTmp = strSrcDir+'/'+ fileInfoList[i].fileName();
            destTmp = strDestDir+'/'+ fileInfoList[i].fileName();
            QFile::copy(srcTmp,destTmp);
        }else if (fileInfoList[i].isDir()) {
            if (QString(".") ==fileInfoList[i].fileName()
                    || QString("..") ==fileInfoList[i].fileName()) {
                continue;
            }
            srcTmp = strSrcDir+'/'+ fileInfoList[i].fileName();
            destTmp = strDestDir+'/'+ fileInfoList[i].fileName();
            copyDir(srcTmp,destTmp);
        }
    }
}

MyTcpSocket::~MyTcpSocket()
{
    // 1. 停止并删除定时器
        if (m_pTimer) {
            m_pTimer->stop();
            delete m_pTimer;
            m_pTimer = nullptr;
        }

        // 2. 关闭并删除文件（如果还开着）
        if (m_file.isOpen()) {
            m_file.close();
        }

        // 3. 释放分块缓冲区
        if (m_pBuffer != nullptr) {
            delete[] m_pBuffer;
            m_pBuffer = nullptr;
        }

        qDebug() << "MyTcpSocket destroyed.";
}

void MyTcpSocket::recvMsg()
{
    if (!m_bUpload) {


        qDebug()<< this->bytesAvailable();
        uint uiPDULen = 0;
        this->read((char*)&uiPDULen,sizeof(uint));
        uint uiMsgLen =uiPDULen - sizeof(PDU);
        PDU *pdu= mkPDU(uiMsgLen);
        this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof (uint));

        qDebug()<<"=================TYPE=="<<pdu->uiMsgType<<(char*)(pdu->caMsg);
        switch (pdu->uiMsgType) {
            case ENUM_MSG_TYPE_REGIST_REQUEST: {
                char caName[32]={'\0'};
                char caPwd[32]={'\0'};
                strncpy(caName,pdu->caData,32);
                strncpy(caPwd,pdu->caData+32,32);
                bool  ret= OpeDB::getInstance().handleRegist(caName,caPwd);
                PDU *respdu=mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
                if (ret) {
                    strcpy(respdu->caData,REGIST_OK);
                    QDir dir;
                qDebug()<<"create dir::"<< dir.mkdir(QString("./%1").arg(caName));


                }else {
                    strcpy(respdu->caData,REGIST_FAILED);
                }
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu=NULL;
                break;
            }
        case ENUM_MSG_TYPE_LOGIN_REQUEST: {
            char caName[32]={'\0'};
            char caPwd[32]={'\0'};
            strncpy(caName,pdu->caData,32);
            strncpy(caPwd,pdu->caData+32,32);
            qDebug()<<caName<<caPwd;
            bool ret=OpeDB::getInstance().handleLogin(caName,caPwd);
            PDU *respdu=mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
            if (ret) {

                strcpy(respdu->caData,LOGIN_OK);
                m_strName=caName;
            }else {
                strcpy(respdu->caData,LOGIN_FAILED);
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST: {
            qDebug()<<"all online";
            QStringList ret=OpeDB::getInstance().handleAllOnline();
            uint uiMsgLen = ret.size() * 32;
            PDU *resPdu=mkPDU(uiMsgLen);
            resPdu->uiMsgType=ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for (int i=0;i<ret.size();i++) {
                memcpy((char*)(resPdu->caMsg)+i*32,ret[i].toStdString().c_str(),ret[i].size());
            }
            write((char*)resPdu,resPdu->uiPDULen);
            free(resPdu);
            resPdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_REQUEST: {
                   int ret= OpeDB::getInstance().handleSearchUsr(pdu->caData);
                   PDU *respdu =mkPDU(0);
                    respdu->uiMsgType=ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
                   if (-1 ==ret) {
                        strcpy(respdu->caData,SEARCH_USR_NO);
                   }else if (0 == ret) {
                       strcpy(respdu->caData,SEARCH_USR_OFFLINE);
                   }else if (1 ==ret) {
                       strcpy(respdu->caData,SEARCH_USR_ONLINE);
                   }
                   write((char*)respdu,respdu->uiPDULen);
                   free(respdu);
                   respdu=NULL;
                break;
            }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: {

            char caPerName[32]={'\0'};
            char caName[32]={'\0'};
            strncpy(caPerName,pdu->caData,32);
            strncpy(caName,pdu->caData+32,32);
            int ret=OpeDB::getInstance().handleAddFriend(caPerName,caName);
    //        qDebug()<<caName<<caPwd;
            qDebug()<<"ADDret:"<<ret;
            PDU *respdu =NULL;
            if (-2==ret) {
                respdu=mkPDU(0);
                respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,ADD_FRIEND_SELF);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu=NULL;
            }else if (-1 == ret) {
                respdu=mkPDU(0);
                respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,UNKNOW_ERROR);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu=NULL;
            }else if (0==ret) {
                respdu=mkPDU(0);
                respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,EXISTED_FRIEND);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu=NULL;
            }else if (1==ret) {
                MyTcpServer::getInstance().resend(caPerName,pdu);
                qDebug()<<"resend ok!";

    //            respdu = mkPDU(0);
    //            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
    //            strcpy(respdu->caData,EXISTED_FRIEND);

    //            write((char*)respdu,respdu->uiPDULen);
    //            free(respdu);
    //            respdu=NULL;
            }else if (2==ret) {
                respdu=mkPDU(0);
                respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,ADD_FRIEND_OFFLINE);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu=NULL;
            }else if (3==ret) {
                respdu=mkPDU(0);
                respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,ADD_FRIEND_NO_EXIST);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu=NULL;
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE: {
            char caPerName[32]={'\0'};
            char caName[32]={'\0'};
            strncpy(caPerName,pdu->caData,32);
            strncpy(caName,pdu->caData+32,32);
            int ret=OpeDB::getInstance().handleAddFriendAge(caPerName,caName);
            //        qDebug()<<caName<<caPwd;
            qDebug()<<"ADDret:"<<ret;
            PDU *respdu =NULL;
            respdu=mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData,"1");
            strncpy(respdu->caData+32,caPerName,32);
            MyTcpServer::getInstance().resend(caName,respdu);
    //        write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE: {
            char caPerName[32]={'\0'};
            char caName[32]={'\0'};
            strncpy(caPerName,pdu->caData,32);
            strncpy(caName,pdu->caData+32,32);
    //        int ret=OpeDB::getInstance().handleAddFriend(caPerName,caName);
            //        qDebug()<<caName<<caPwd;
    //        qDebug()<<"ADDret:"<<ret;
            PDU *respdu =NULL;
            respdu=mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData,"0");
            strncpy(respdu->caData+32,caPerName,32);
            MyTcpServer::getInstance().resend(caName,respdu);
    //        write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST: {
            char caName[32]={'\0'};
            strncpy(caName,pdu->caData,32);
            QStringList ret=OpeDB::getInstance().handleFlushFriend(caName);
            uint uiMsgLen=ret.size()*32;
            PDU *respdu =mkPDU(uiMsgLen);
            respdu->uiMsgType=ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
            for (int i=0;i<ret.size();i++) {
                memcpy((char*)(respdu->caMsg)+i*32,
                       ret.at(i).toStdString().c_str(),
                       ret.at(i).size());
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_DEL_FRIEND_REQUEST: {
            char caSelfName[32]={'\0'};
            char caFriendName[32]={'\0'};
            strncpy(caSelfName,pdu->caData,32);
            strncpy(caFriendName,pdu->caData+32,32);
            OpeDB::getInstance().handleDelFriend(caSelfName,caFriendName);

            PDU *respdu =mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FRIEND_RESPOND;
            strcpy(respdu->caData,DEL_FRIEND_OK);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;

            MyTcpServer::getInstance().resend(caFriendName,pdu);

            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST: {
            char caPerName[32]={'\0'};
            memcpy(caPerName,pdu->caData+32,32);
            qDebug()<<"IN PC:"<<caPerName;
            MyTcpServer::getInstance().resend(caPerName,pdu);

            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST: {
            char caName[32]={'\0'};
            strncpy(caName,pdu->caData,32);
            QStringList onlineFriend=OpeDB::getInstance().handleFlushFriend(caName);
            QString tmp;
            for (int i=0;i<onlineFriend.size();i++) {
                tmp=onlineFriend.at(i);
                 MyTcpServer::getInstance().resend(tmp.toStdString().c_str(),pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST: {
            qDebug()<<"::IN_CRE_DIR::";
            QDir dir;
            QString strCurPath =QString("%1").arg((char*)(pdu->caMsg));
            bool ret =dir.exists(strCurPath);
            qDebug()<<strCurPath;
            PDU *respdu =nullptr;
            if (ret) {
                char caNewDir[32]={'\0'};
                memcpy(caNewDir,pdu->caData+32,32);
                QString strNewPath = strCurPath+"/"+caNewDir;
                qDebug() << strNewPath;
                ret=dir.exists(strNewPath);
                qDebug()<<ret;
                if (ret) {
                    respdu=mkPDU(0);
                    strcpy(respdu->caData,FILE_NAME_EXIST);
                    respdu->uiMsgType=ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                }else {
                    dir.mkdir(strNewPath);
                    respdu=mkPDU(0);
                    strcpy(respdu->caData,CREATE_DIR_OK);
                    respdu->uiMsgType=ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                }
            }else {
                respdu=mkPDU(0);
                strcpy(respdu->caData,DIR_NO_EXIST);
                respdu->uiMsgType=ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=nullptr;
            break;

        }
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST: {
            char *pCurPath =new  char[pdu->uiMsgLen];
            memcpy(pCurPath, pdu->caMsg, pdu->uiMsgLen);
            QDir dir(pCurPath);
            QFileInfoList fileInfoList=dir.entryInfoList();
            int iFileCount =fileInfoList.size();
            PDU *respdu=mkPDU(sizeof(FileInfo)*(iFileCount-2));
            respdu->uiMsgType=ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;

            FileInfo *pFileInfo=nullptr;
            QString strFileName;
            int idx=0;
            for (int i=0;i<iFileCount;i++) {
                if (QString(".")==fileInfoList[i].fileName()
                        || QString("..") == fileInfoList[i].fileName()) {
                    continue;
                }
                qDebug()<<fileInfoList[i].fileName()<<fileInfoList[i].size()
                       <<"文件夹："<<fileInfoList[i].isDir()
                      << "常规文件："<<fileInfoList[i].isFile();

                pFileInfo =(FileInfo*)(respdu->caMsg)+(idx++);
                strFileName=fileInfoList[i].fileName();

                memcpy(pFileInfo->caName,strFileName.toStdString().c_str(),strFileName.size());

                if (fileInfoList[i].isDir()) {
                    pFileInfo->iFileType=0;
                }else if (fileInfoList[i].isFile()) {
                    pFileInfo->iFileType=1;
                }
            }
            qDebug()<<"::WRI_BEF_FF::";
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=nullptr;
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_REQUEST: {

            char caName[32]={'\0'};
            strcpy(caName,pdu->caData);
            char *pPath =new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath=QString("%1/%2").arg(pPath).arg(caName);
            qDebug()<<strPath;

            QFileInfo fileInfo(strPath);
            bool ret =false;
            if (fileInfo.isDir()) {
               QDir dir;
               dir.setPath(strPath);
                dir.removeRecursively();
                ret=true;
            }else if (fileInfo.isFile()) {
                ret=false;
            }else {

            }
            PDU *respdu= nullptr;
            if (ret) {
                respdu=mkPDU(strlen(DEL_DIR_OK)+1);
                respdu->uiMsgType=ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(respdu->caData,DEL_DIR_OK,strlen(DEL_DIR_OK));
            }else {
                respdu=mkPDU(strlen(DEL_DIR_OK)+1);
                respdu->uiMsgType=ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(respdu->caData,DEL_DIR_FAILURED,strlen(DEL_DIR_FAILURED));
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=nullptr;
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_REQUEST: {
            char caOldName[32]={'\0'};
            char caNewName[32]={'\0'};
            strncpy(caOldName,pdu->caData,32);
            strncpy(caNewName,pdu->caData+32,32);

            char *pPath =new char[pdu->uiMsgLen+1];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);

            QString strOldPath=QString ("%1/%2").arg(pPath).arg(caOldName);
            QString strNewPath=QString ("%1/%2").arg(pPath).arg(caNewName);
            qDebug()<<strOldPath<<strNewPath;
            QDir dir;
            bool ret = dir.rename(strOldPath, strNewPath);
            PDU *respdu=mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
            if (ret) {
                strcpy(respdu->caData,RENAME_FILE_OK);
            }else {
                strcpy(respdu->caData,RENAME_FILE_FAILURED);
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=nullptr;


            break;
        }
        case ENUM_MSG_TYPE_ENTER_FILE_REQUEST: {
            char caEnterName[32]={'\0'};
            strncpy(caEnterName,pdu->caData,32);

            char *pPath =new char[pdu->uiMsgLen+1];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);

            QString strPath=QString ("%1/%2").arg(pPath).arg(caEnterName);
            qDebug()<<strPath;

            QFileInfo fileInfo(strPath);
            PDU *respdu=nullptr;
            if (fileInfo.isDir()) {
                qDebug()<<"in enter dir:";
                QDir dir(strPath);
                QFileInfoList fileInfoList=dir.entryInfoList();
                int iFileCount =fileInfoList.size();
                respdu=mkPDU(sizeof(FileInfo)*(iFileCount-2));
    //            respdu=mkPDU(sizeof(FileInfo)*(iFileCount));
                respdu->uiMsgType=ENUM_MSG_TYPE_ENTER_FILE_RESPOND;
                strncpy(respdu->caData,ENTER_DIR_OK,strlen(ENTER_DIR_OK));
                FileInfo *pFileInfo=nullptr;
                QString strFileName;
                int idx=0;
                for (int i=0;i<iFileCount;i++) {
                   if (QString(".")==fileInfoList[i].fileName()
                            || QString("..") == fileInfoList[i].fileName()) {
                        continue;
                    }
                    qDebug()<<fileInfoList[i].fileName()<<fileInfoList[i].size()
                           <<"文件夹："<<fileInfoList[i].isDir()
                          << "常规文件："<<fileInfoList[i].isFile();

                    pFileInfo =(FileInfo*)(respdu->caMsg)+(idx++);
                    strFileName=fileInfoList[i].fileName();

                    memcpy(pFileInfo->caName,strFileName.toStdString().c_str(),strFileName.size());

                    if (fileInfoList[i].isDir()) {
                        pFileInfo->iFileType=0;
                    }else if (fileInfoList[i].isFile()) {
                        pFileInfo->iFileType=1;
                    }
                }
                qDebug()<<"::enter::";
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu=nullptr;


    //            respdu=mkPDU(0);
    //            respdu->uiMsgType=ENUM_MSG_TYPE_ENTER_FILE_RESPOND;
    //            strncpy(respdu->caData,ENTER_DIR_OK,strlen(ENTER_DIR_OK));

    //            write((char*)respdu,respdu->uiPDULen);
    //            free(respdu);
    //            respdu=nullptr;
            }else if (fileInfo.isFile()) {
                respdu=mkPDU(0);
                respdu->uiMsgType=ENUM_MSG_TYPE_ENTER_FILE_RESPOND;
                strncpy(respdu->caData,ENTER_DIR_FAILURED,strlen(ENTER_DIR_FAILURED));

                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu=nullptr;
            }

            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:{
            char caFileName[32]={'\0'};
            qint64 fileSize =0;
            sscanf(pdu->caData,"%s %lld",caFileName,&fileSize);
    //        strncpy(caFileName,pdu->caData,32);

            char *pPath =new char[pdu->uiMsgLen+1];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);

            QString strPath=QString ("%1/%2").arg(pPath).arg(caFileName);
            qDebug()<<strPath;
            delete [] pPath;
            pPath =nullptr;

            m_file.setFileName(strPath);
            qDebug()<<"BEF::::"<<caFileName;
            if (m_file.open(QIODevice::WriteOnly)) {
                m_bUpload=true;
                m_iTotal =fileSize;
                m_iRecved =0;
                qDebug()<<"ok "<<m_iTotal<<m_bUpload;
            }else {
                qDebug()<<"false";
            }
            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_REQUEST :{

            char caName[32]={'\0'};
            strcpy(caName,pdu->caData);
            char *pPath =new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath=QString("%1/%2").arg(pPath).arg(caName);
            qDebug()<<strPath;

            QFileInfo fileInfo(strPath);
            bool ret =false;
            if (fileInfo.isDir()) {

                ret=false;
            }else if (fileInfo.isFile()) {
                QDir dir;
                ret=dir.remove(strPath);

            }else {

            }
            PDU *respdu= nullptr;
            if (ret) {
                respdu=mkPDU(strlen(DEL_FILE_OK)+1);
                respdu->uiMsgType=ENUM_MSG_TYPE_DEL_FILE_RESPOND;
                memcpy(respdu->caData,DEL_FILE_OK,strlen(DEL_FILE_OK));
            }else {
                respdu=mkPDU(strlen(DEL_FILE_FAILURED)+1);
                respdu->uiMsgType=ENUM_MSG_TYPE_DEL_FILE_RESPOND;
                memcpy(respdu->caData,DEL_FILE_FAILURED,strlen(DEL_FILE_FAILURED));
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=nullptr;
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST: {
            char caFileName[32]={'\0'};
            strcpy(caFileName,pdu->caData);
    //        strncpy(caFileName,pdu->caData,32);

            char *pPath =new char[pdu->uiMsgLen+1];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);

            QString strPath=QString ("%1/%2").arg(pPath).arg(caFileName);
            qDebug()<<strPath;
            delete [] pPath;
            pPath =nullptr;

            QFileInfo fileInfo(strPath);
            qint64 fileSize =fileInfo.size();

            PDU *respdu= mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
            sprintf(respdu->caData,"%s %lld",caFileName, fileSize);

            qDebug()<<caFileName<<fileSize;

            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu=nullptr;



//            m_file.setFileName(strPath);
//            m_file.open(QIODevice::ReadOnly);
//            m_pTimer->start(1000);

            // ✅ 准备分块发送
            m_file.setFileName(strPath);
            if (m_file.open(QIODevice::ReadOnly)) {
              // 分配缓冲区（如果之前没分配）
              if (m_pBuffer == nullptr) {
                  m_pBuffer = new char[BLOCK_SIZE];
              }
              // 启动定时器，开始分块发送
              m_pTimer->start(100); // 每10ms发送一块，可根据网络情况调整
            } else {
              qDebug() << "Failed to open file for reading:" << strPath;
            }

            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST: {
            char caSendName[32]={'\0'};
            int num=0;
            sscanf(pdu->caData,"%s %d",caSendName,&num);
            int size=num*32;

            PDU *respdu =mkPDU(pdu->uiMsgLen-size);
            respdu->uiMsgType=ENUM_MSG_TYPE_SHARE_FILE_NOTE;
            strcpy(respdu->caData,caSendName);
            memcpy((char*)(respdu->caMsg),(char*)(pdu->caMsg)+size, pdu->uiMsgLen-size);

            char caRecvName[32]={'\0'};
            for (int i=0;i<num;i++) {
                memcpy(caRecvName, (char*)(pdu->caMsg)+32*i,32);
                MyTcpServer::getInstance().resend(caRecvName,respdu);
            }
            free(respdu);
            respdu =nullptr;

            respdu=mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
            strcpy(respdu->caData,SHARE_FILE_OK );
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;

            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND:{
//            char caRecvPath[32]={'\0'};
            qDebug()<<"====in SHARE_NOT_REPOD";
            QString strRecvPath =QString("./%1").arg(pdu->caData);
            QString strShareFilePath =QString("%1").arg((char*)pdu->caMsg);
            int index=strShareFilePath.lastIndexOf('/');
            QString strFileName =strShareFilePath.right(strShareFilePath.size()-index-1);
            strRecvPath = strRecvPath+'/'+strFileName;
            qDebug()<<"strShareFilePath:"<<strShareFilePath<<"\n index:"<<index;
            qDebug()<<"recv:"<<strRecvPath<<"\n share:"<<strFileName;

            QFileInfo fileInfo(strShareFilePath);
            if (fileInfo.isFile()) {
                QFile::copy(strShareFilePath,strRecvPath);
            }else if (fileInfo.isDir()) {
                copyDir(strShareFilePath,strRecvPath);
            }

            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:{
            qDebug()<<"=====IN MOVE REP";
            char  caFileName[32]={'\0'};
            int srcLen =0;
            int destLen =0;
            sscanf(pdu->caData,"%d %d %s",&srcLen, &destLen,caFileName);
            char *pSrcPath =new char[srcLen+1];
            char *pDestPath =new char[destLen+1+32];
            memset(pSrcPath,'\0',srcLen+1);
            memset(pDestPath,'\0',destLen+1);

            memcpy(pSrcPath,pdu->caMsg,srcLen);
            memcpy(pDestPath,(char*)(pdu->caMsg)+(srcLen+1),destLen);

            PDU *respdu= mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;
            QFileInfo fileInfo(pDestPath);

            qDebug()<<"pSrcPath::"<<pSrcPath<<"pDestPath::"<<pDestPath;
            if (fileInfo.isDir()) {
                strcat(pDestPath,"/");
                strcat(pDestPath,caFileName);
                strcat(pSrcPath,"/");
                strcat(pSrcPath,caFileName);
                qDebug()<<"pSrcPath::"<<pSrcPath<<"pDestPath::"<<pDestPath;
                bool ret =  QFile::rename(pSrcPath,pDestPath);
                qDebug()<<"RET::"<<ret;
                if (ret) {
                    strcpy(respdu->caData,MOVE_FILE_OK);
                }else {
                    strcpy(respdu->caData,COMMON_ERR);
                }
            }else if (fileInfo.isFile()) {
                strcpy(respdu->caData,MOVE_FILE_FAILURED);
            }

            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=nullptr;
            break;
        }
        default:
                break;
        }

        free(pdu);
        pdu=NULL;
    }else {
        qDebug()<<"NOW:::UPC";

        PDU *respdu = nullptr;
        respdu = mkPDU(0);
        respdu->uiMsgType =ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;

        QByteArray buff= readAll();
        m_file.write(buff);
        m_iRecved +=buff.size();
        if (m_iTotal == m_iRecved) {
            m_file.close();
            m_bUpload =false;

            strcpy(respdu->caData,UPLOAD_FILE_OK);

        }else if (m_iTotal < m_iRecved) {
            m_file.close();
            m_bUpload =false;
            strcpy(respdu->caData,UPLOAD_FILE_FAILURED);
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=nullptr;
    }
//    qDebug()<<caName<<caPwd<<pdu->uiMsgType;

}

void MyTcpSocket::clientOffine()
{
    OpeDB::getInstance().handleOffine(m_strName.toStdString().c_str());
    emit offline(this);
    qDebug() << "clientOffine triggered, user =" << m_strName;

}

void MyTcpSocket::sendFileToClient()
{
    m_pTimer->stop();
    char *pData =new char[4096];
    qint64 ret=0;
    while (true) {
        ret =m_file.read(pData,4096);
        if (ret>0 && ret<=4096) {
            write(pData,ret);
        }else if (0== ret) {
            m_file.close();
            break;
        }else if (ret<0) {
            qDebug()<<"发送文件过程中失败";
            m_file.close();
            break;
        }
    }
    delete []pData;
    pData = nullptr;

    // 只有在发送文件时才处理
//        if (!m_file.isOpen()) {
//            return;
//        }

//        // 读取一个数据块
//        qint64 ret = m_file.read(m_pBuffer, BLOCK_SIZE);

//        if (ret > 0) {
//            // 发送这个数据块
//            write(m_pBuffer, ret);
//            // write() 是异步的，Qt会自动处理套接字就绪事件
//        }
//        else if (ret == 0) {
//            // 文件已读完
//            m_file.close();
//            m_pTimer->stop(); //  停止定时器
//            qDebug() << "File transfer completed.";

//            //发送一个结束信号给客户端
//            // PDU *endPdu = mkPDU(0);
//            // endPdu->uiMsgType = ENUM_MSG_TYPE_FILE_TRANSFER_END;
//            // write((char*)endPdu, endPdu->uiPDULen);
//            // free(endPdu);
//        }
//        else {
//            // 读取错误
//            qDebug() << "Error reading file:" << m_file.errorString();
//            m_file.close();
//            m_pTimer->stop();
//        }

}
