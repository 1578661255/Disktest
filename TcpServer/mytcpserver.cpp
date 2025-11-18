#include "mytcpserver.h"
#include <QDebug>
#include <QList>
#include "mytcpsocket.h"

MyTcpServer::MyTcpServer()
{

}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug()<< "new client connected";
    MyTcpSocket *pTcpSocket= new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    m_tcpSocketList.append(pTcpSocket);

    connect(pTcpSocket,SIGNAL(offline(MyTcpSocket*)),
            this,SLOT(deleteSocket(MyTcpSocket*)));

}

void MyTcpServer::resend(const char *pername, PDU *pdu)
{
    qDebug()<< "in resend:";
    if (NULL==pername || NULL==pdu) {
        return ;
    }
    QString strName =pername;
    for (int i=0;i<m_tcpSocketList.size();i++) {
        QString tmp=m_tcpSocketList.at(i)->getName();
        qDebug()<<tmp;
        if (strName == tmp ) {
//            qDebug() << "write len =" << pdu->uiPDULen;

            m_tcpSocketList.at(i)->write((char*)pdu,pdu->uiPDULen);
            qDebug()<<"resend out";
            break;
        }
    }
}

void MyTcpServer::deleteSocket(MyTcpSocket *mysocket)
{
       QList<MyTcpSocket*>::iterator iter=m_tcpSocketList.begin();
       for (;iter!=m_tcpSocketList.end();iter++) {
            if (mysocket == *iter) {
//                delete *iter;
//                *iter=NULL;
                m_tcpSocketList.erase(iter);   // 移出列表
                mysocket->deleteLater();
//                m_tcpSocketList.erase(iter);
                break;
            }
       }
       for (auto x:m_tcpSocketList) {
           qDebug()<<x->getName();
       }
}
