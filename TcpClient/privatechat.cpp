#include "privatechat.h"
#include "ui_privatechat.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QMessageBox>
#include <QString>

PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

void PrivateChat::setChatName(QString strName)
{
    m_strChatName=strName;
    m_strLoginName=TcpClient::getInstance().loginName();
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance;
    return instance;
}

void PrivateChat::updateMsg(const PDU *pdu)
{
    qDebug()<<"updateMsg()::";
    if (NULL ==pdu) {
        return ;
    }
    char caSendName[32]={'\0'};
    memcpy(caSendName,pdu->caData,32);
    qDebug()<<"caSendName:"<<caSendName;
    QString strMsg=QString("%1 say %2").arg(caSendName).arg((char *)pdu->caMsg);
    ui->showMsg_te->append(strMsg);
}

void PrivateChat::on_sendMsg_pb_clicked()
{
    QString strMsg=ui->inputMsg_lw->text();
    ui->inputMsg_lw->clear();
    if (!strMsg.isEmpty()) {
        PDU *pdu=mkPDU(strMsg.size()+1);
        pdu->uiMsgType=ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;

        memcpy(pdu->caData,m_strLoginName.toStdString().c_str(),m_strLoginName.size());
        memcpy(pdu->caData+32,m_strChatName.toStdString().c_str(),m_strChatName.size());
//        strcpy()
//        strcpy(pdu->caMsg,strMsg.toStdString().c_str());
        memcpy(pdu->caMsg, strMsg.toStdString().c_str(), strMsg.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;

    }else {
        QMessageBox::warning(this,"私聊","发送聊天信息不为空");
    }
}
