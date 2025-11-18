#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QScrollArea>
#include <QListWidget>
#include "opewidget.h"
class ShareFile : public QWidget
{
    Q_OBJECT
public:
    explicit ShareFile(QWidget *parent = nullptr);

    static ShareFile &getInstance();

    void test();

    void updateFriend(QListWidget *pFriendList);
signals:
public slots:
    void cancelSelect();
    void selectAll();

    void okShare();
    void cancelShare();
//    void
private:
    QPushButton *m_pSelectAllPB;
    QPushButton *m_pCancleSelectAllPB;

    QPushButton *m_pOKPB;
    QPushButton *m_pCanclePB;

    QScrollArea *m_pSA;
    QWidget *m_pFriendW;
    QVBoxLayout *m_pFriendWVBL;
    QButtonGroup *m_pButtonGroup;
};

#endif // SHAREFILE_H
