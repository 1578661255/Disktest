#include "sharefile.h"
#include <QCheckBox>
#include <QDebug>
#include "tcpclient.h"

ShareFile::ShareFile(QWidget *parent) : QWidget(parent)
{
    m_pSelectAllPB = new QPushButton("全选");
    m_pCancleSelectAllPB = new QPushButton("取消选择");

    m_pOKPB = new QPushButton("确定");
    m_pCanclePB = new QPushButton("取消");

    m_pSA = new QScrollArea;
    m_pFriendW = new QWidget;

    m_pFriendWVBL = new QVBoxLayout(m_pFriendW);

    m_pButtonGroup = new QButtonGroup(m_pFriendW);
    m_pButtonGroup->setExclusive(false);

//    m_pFriendW->setLayout(m_pFriendWVBL);

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancleSelectAllPB);
    pTopHBL->addStretch();

    QHBoxLayout *pDownHBL = new QHBoxLayout;
    pDownHBL->addWidget(m_pOKPB);
    pDownHBL->addWidget(m_pCanclePB);
//    pDownHBL->addStretch();
    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pSA);
    pMainVBL->addLayout(pDownHBL);
    setLayout(pMainVBL);

//    test();
    connect(m_pCancleSelectAllPB,&QPushButton::clicked,
            this,&ShareFile::cancelSelect);
    connect(m_pSelectAllPB,&QPushButton::clicked,
            this,&ShareFile::selectAll);
    connect(m_pOKPB,&QPushButton::clicked,
            this,&ShareFile::okShare);
    connect(m_pCanclePB,&QPushButton::clicked,
            this,&ShareFile::cancelShare);
}

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;
}

void ShareFile::test()
{
//    QVBoxLayout *p =new QVBoxLayout(m_pFriendW);
    QCheckBox *pCB= nullptr;
    for (int i=0;i<15;i++) {
        pCB =new QCheckBox("abv");
        m_pFriendWVBL->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);
    }
    m_pSA->setWidget(m_pFriendW);

}

void ShareFile::updateFriend(QListWidget *pFriendList)
{
    qDebug()<<"-------IN UPDATE::";
    if (nullptr == pFriendList) {
        return ;
    }

//    QAbstractButton *tmp=nullptr;
    QList<QAbstractButton*> preFriendList = m_pButtonGroup->buttons();
    qDebug()<<"========pFriendList";
    for (int i=0;i<pFriendList->count();i++) {
       qDebug()<<pFriendList->item(i)->text();
    }
    qDebug()<<"========pFriendList";
    qDebug()<<"========preFriendList";
    for (int i=0;i<preFriendList.size();i++) {
       qDebug()<<preFriendList[i]->text();
    }
    qDebug()<<"========preFriendList";
      for (int i = preFriendList.size() - 1; i >= 0; --i) {
          QAbstractButton* btn = preFriendList[i];
          m_pFriendWVBL->removeWidget(btn);
          m_pButtonGroup->removeButton(btn);
          delete btn; // 手动释放，避免泄漏
      }
    QCheckBox *pCB= nullptr;
    for (int i=0;i<pFriendList->count();i++) {
        pCB =new QCheckBox(pFriendList->item(i)->text());
        m_pFriendWVBL->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);
        qDebug()<<"m_pButtonGroup->addButton()"<<pCB->text();
    }


    m_pSA->setWidget(m_pFriendW);
    m_pSA->setWidgetResizable(true); //
    m_pFriendW->adjustSize();
    qDebug()<<"-----------OUT UPDATE::";

}

void ShareFile::cancelSelect()
{
    QList<QAbstractButton*> cbList= m_pButtonGroup->buttons();
    for (int i=0;i<cbList.size();i++) {
        if (cbList[i]->isChecked()) {
            cbList[i]->setChecked(false);
        }
    }

}

void ShareFile::selectAll()
{
    QList<QAbstractButton*> cbList= m_pButtonGroup->buttons();
    for (int i=0;i<cbList.size();i++) {
        if (!cbList[i]->isChecked()) {
            cbList[i]->setChecked(true);
        }
    }
}

void ShareFile::okShare()
{
    QString strName = TcpClient::getInstance().loginName();
    QString strCurPath = TcpClient::getInstance().curPath();
    QString strShareFileName =OpeWidget::getInstance().getBook()->getShareFileName();

    QString strPath =strCurPath+"/"+strShareFileName;

    QList<QAbstractButton*> cbList= m_pButtonGroup->buttons();
    int num=0;
    for (int i=0;i<cbList.size();i++) {
        if (cbList[i]->isChecked()) {
            num++;
        }
    }

    PDU *pdu =mkPDU(32*num+strPath.size()+1);
    pdu->uiMsgType =ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData,"%s %d",strName.toStdString().c_str(),num);

    int j=0;
    for (int i=0;i<cbList.size();i++) {
        if (cbList[i]->isChecked()) {
            memcpy((char*)(pdu->caMsg)+j*32,cbList[i]->text().toStdString().c_str(),cbList[i]->text().size());
            j++;
        }

    }

    memcpy((char*)(pdu->caMsg)+num*32,strPath.toStdString().c_str(),strPath.size());

    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=nullptr;
}

void ShareFile::cancelShare()
{
    hide();
}
