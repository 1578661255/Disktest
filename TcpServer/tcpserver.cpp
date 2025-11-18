#include "tcpserver.h"
#include "ui_tcpserver.h"
#include "mytcpserver.h"
#include <QByteArray>
#include <QtDebug>
#include <QMessageBox>
#include <QHostAddress>
#include <QFile>


TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    loadConfig();
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);
}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    QFile file(":/server.config");
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

