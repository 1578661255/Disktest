#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QByteArray>
#include <QtDebug>
#include <QMessageBox>
#include <QHostAddress>
#include "protocol.h"
#include "privatechat.h"
#include <QString>


TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);

    resize(500,350);

    loadCongig();
    connect(&m_tcpsocket,SIGNAL(connected())
            ,this, SLOT(showConnect()));
    connect(&m_tcpsocket,SIGNAL(readyRead())
            ,this, SLOT(recvMsg()));
    m_tcpsocket.connectToHost(QHostAddress(m_strIP),m_usPort);


}

TcpClient::~TcpClient()
{
    delete ui;
}


void TcpClient::loadCongig()
{
    QFile file(":/client.config");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray baData = file.readAll();
//        QString strData = baData.toStdString().c_str();
        QString strData =QString(baData);
        strData.replace("\r\n"," ");
        QStringList strList=strData.split(" ");

        m_strIP =strList[0];
        m_usPort =strList.at(1).toUShort();
        qDebug() << m_strIP<<m_usPort;
        file.close();

    }else {
        QMessageBox::critical(this,"open config","open config fail");
    }
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return  instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpsocket;
}

QString TcpClient::loginName()
{
    return m_strLoginName;
}

QString TcpClient::curPath()
{
    return m_strCurPath;
}

void TcpClient::setCurPath(QString strCurPath)
{
    m_strCurPath = strCurPath;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this,"connect server","successfully connected");

}

void TcpClient::recvMsg()
{
    if (!OpeWidget::getInstance().getBook()->getDownloadStatus()) {
        qDebug()<< m_tcpsocket.bytesAvailable();
        uint uiPDULen = 0;
        m_tcpsocket.read((char*)&uiPDULen,sizeof(uint));
        uint uiMsgLen =uiPDULen - sizeof(PDU);
        PDU *pdu= mkPDU(uiMsgLen);
        m_tcpsocket.read((char*)pdu+sizeof(uint),uiPDULen-sizeof (uint));

         qDebug()<<"=================TYPE=="<<pdu->uiMsgType<<(char*)(pdu->caMsg);
        switch (pdu->uiMsgType) {
            case ENUM_MSG_TYPE_REGIST_RESPOND: {
                if (0== strcmp(pdu->caData,REGIST_OK)) {
                    QMessageBox::information(this,"regist","regist ok");
                }else if (0== strcmp(pdu->caData,REGIST_FAILED)) {
                    QMessageBox::information(this,"regist",REGIST_FAILED);
                }
                break;
            }
        case ENUM_MSG_TYPE_LOGIN_RESPOND: {
            if (0== strcmp(pdu->caData,LOGIN_OK)) {
                m_strCurPath= QString("./%1").arg(m_strLoginName);
                QMessageBox::information(this,"login",LOGIN_OK);
                OpeWidget::getInstance().show();
                   this->hide();

            }else if (0== strcmp(pdu->caData,LOGIN_FAILED)) {
                QMessageBox::information(this,"login",LOGIN_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND: {
            OpeWidget::getInstance().getFriend()->showAllOnlineUsr(pdu);
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_RESPOND: {
            if (0 == strcmp(SEARCH_USR_NO,pdu->caData)) {
               QMessageBox::information(this,"搜索",QString("%1: not exist").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));

            }else if (0 == strcmp(SEARCH_USR_OFFLINE,pdu->caData)) {
                QMessageBox::information(this,"搜索",QString("%1: offline").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));

            }else if (0 == strcmp(SEARCH_USR_ONLINE,pdu->caData)) {
                QMessageBox::information(this,"搜索",QString("%1: online").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));

            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: {
            char caName[32]={'\0'};
            strncpy(caName,pdu->caData+32,32);

           int ret = QMessageBox::information(this,"添加好友",QString("%1 want add u as friend?").arg(caName),
                                     QMessageBox::Yes,QMessageBox::No);
           qDebug()<<"ret::"<<ret;
           PDU *respdu =mkPDU(0);
           memcpy(respdu->caData,pdu->caData,32);
           memcpy(respdu->caData+32,pdu->caData+32,32);
           QString perName=loginName();
           qDebug()<<"RES::"<<respdu->caData<<respdu->caData+32;
           if (QMessageBox::Yes == ret) {
                respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_AGGREE;

            }else {
               respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;

            }
           m_tcpsocket.write((char *)respdu,respdu->uiPDULen);
           free(respdu);
           respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND: {
            char perName[32]={'\0'};
            strcpy(perName,pdu->caData+32);
    //        QMessageBox::information(this,"添加哈偶",pdu->caData);
            if (pdu->caData[0] == '1') {
                QMessageBox::information(this,"添加好友",QString("%1同意").arg(perName));
            }else if (pdu->caData[0] == '0') {
                QMessageBox::information(this,"regist",QString("%1拒绝").arg(perName));
            }
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND: {
               OpeWidget::getInstance().getFriend()->updateFriendList(pdu);
            break;
        }

        case ENUM_MSG_TYPE_DEL_FRIEND_REQUEST: {
            char caName[32]="\0";
            memcpy(caName,pdu->caData,32);
            QMessageBox::information(this,"删除",QString("%1 删了你").arg(caName));
            break;
        }
        case ENUM_MSG_TYPE_DEL_FRIEND_RESPOND: {
            QMessageBox::information(this,"删除","删除成功");
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST: {
            qDebug()<<"IN PC_RQ";
            if (PrivateChat::getInstance().isHidden()) {
                  PrivateChat::getInstance().show();
            }
                char caSendName[32]={'\0'};
                memcpy(caSendName,pdu->caData,32);
                QString strSendName=caSendName;
                PrivateChat::getInstance().setChatName(strSendName);

                PrivateChat::getInstance().updateMsg(pdu);


            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST: {
            OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu);


            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND: {
            QMessageBox::information(this,"创建文件夹",pdu->caData);


            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND: {
            OpeWidget::getInstance().getBook()->updateFleList(pdu);

            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_RESPOND: {
            QMessageBox::information(this,"删除文件夹",pdu->caData);

            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_RESPOND: {
            QMessageBox::information(this,"重命名文件",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ENTER_FILE_RESPOND: {
            QString ret=pdu->caData;
            qDebug()<<"::IN_ENTER_FILE::"<<ret;
            if (ret== ENTER_DIR_FAILURED) {
    //             OpeWidget::getInstance().getBook()->clearEnterDir();
                 QMessageBox::information(this,"进入文件",pdu->caData);
            }else if (ret== ENTER_DIR_OK){
    //            QMessageBox::information(this,"进入文件",pdu->caData);
                qDebug()<<"enter Ok::";
                QString strEnterDir=OpeWidget::getInstance().getBook()->getenterDir();
                if (!strEnterDir.isEmpty()) {
                    m_strCurPath =m_strCurPath+"/"+strEnterDir;
                    qDebug()<<"enter dir:"<<m_strCurPath;
                }
                OpeWidget::getInstance().getBook()->updateFleList(pdu);

            }


            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND: {
            QMessageBox::information(this,"上传文件",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_RESPOND: {
             QMessageBox::information(this,"删除文件",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND: {
            qDebug()<<"IN::DOWNFILE";
            qDebug() << pdu->caData;
            char caFileName[32]={'\0'};
            sscanf(pdu->caData,"%s %lld",caFileName,&OpeWidget::getInstance().getBook()->m_iTotal);
            if (strlen(caFileName)>0 && OpeWidget::getInstance().getBook()->m_iTotal>0) {
                OpeWidget::getInstance().getBook()->setDownloadFile(true);
                m_file.setFileName(OpeWidget::getInstance().getBook()->getSaveFilePath());
                if (!m_file.open(QIODevice::WriteOnly)) {
                    QMessageBox::warning(this,"下载文件","获得保存文件路径失败");
                }

            }
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_RESPOND: {
            QMessageBox::information(this,"共享文件",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE: {
            char *pPath =new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            char *pos=strrchr(pPath, '/');
            if (nullptr !=pos) {
                pos++;
                QString strNote = QString("%1 share file->%2 \n do u accept ?").arg(pdu->caData).arg(pos);
                int ret=QMessageBox::question(this,"共享文件",strNote);
                if (QMessageBox::Yes ==ret) {
                    PDU *respdu =mkPDU(pdu->uiMsgLen);
                    respdu->uiMsgType=ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;

                    memcpy(respdu->caMsg,pdu->caMsg,pdu->uiMsgLen);
                    QString strName = TcpClient::getInstance().loginName();

                    strcpy(respdu->caData,strName.toStdString().c_str());
                    m_tcpsocket.write((char*)respdu,respdu->uiPDULen);


                }else {

                }
            }
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_RESPOND: {
            qDebug()<<"=======IN MOVE_FILE_RESPOND";
            QMessageBox::information(this,"移动文件",pdu->caData);
            break;
        }
        default:
            break;
        }

        free(pdu);
        pdu=NULL;
    }else {
        qDebug()<<"======IN::RECV_FILE========";
        QByteArray buffer=m_tcpsocket.readAll();
        m_file.write(buffer);
        Book *pBook = OpeWidget::getInstance().getBook();
        pBook->m_iRecved += buffer.size();
        qDebug()<<pBook->m_iTotal<<pBook->m_iRecved;
        if (pBook->m_iTotal == pBook->m_iRecved) {
            m_file.close();
            pBook->m_iTotal=0;
            pBook->m_iRecved=0;
            pBook->setDownloadFile(false);
            QMessageBox::information(this,"下载文件","下载文件成功");

        }else if(pBook->m_iTotal < pBook->m_iRecved) {
            m_file.close();
            pBook->m_iTotal=0;
            pBook->m_iRecved=0;
            pBook->setDownloadFile(false);

            QMessageBox::critical(this,"下载文件","下载文件失败");

        }
    }
}


void TcpClient::on_send_pd_clicked()
{
    QString strMsg =ui->lineEdit->text();
    if (strMsg.isEmpty()) {
        QMessageBox::warning(this, "msg send","msg send cant be null");
    }else {
        PDU *pdu=mkPDU(strMsg.size()+1);
        pdu->uiMsgType=5341;
        memcpy(pdu->caMsg, strMsg.toStdString().c_str(), strMsg.size());
        qDebug() << (char*)(pdu->caMsg);
        m_tcpsocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }

}

void TcpClient::on_login_pb_clicked()
{
    QString strName=ui->name_le->text();
    QString strPwd=ui->pwd_le->text();
    if (!strPwd.isEmpty() && !strName.isEmpty()) {
//        QString hashedPwd = sha256(strPwd);

        m_strLoginName=strName;
        PDU *pdu =mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        m_tcpsocket.write((char*)pdu,pdu->uiPDULen);
        qDebug()<<strName<<strPwd;
        free(pdu);
        pdu=NULL;
//        QMessageBox::information(this, "login",LOGIN_OK);
    }else {
         QMessageBox::critical(this, "login","login fail:: cant be null");
    }
}

void TcpClient::on_regist_pb_clicked()
{
    QString strName=ui->name_le->text();
    QString strPwd=ui->pwd_le->text();
    if (!strPwd.isEmpty() && !strName.isEmpty()) {
        PDU *pdu =mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        m_tcpsocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;

    }else {
         QMessageBox::critical(this, "regist","regist fail:: cant be null");
    }

}

void TcpClient::on_cancel_pb_clicked()
{

}
