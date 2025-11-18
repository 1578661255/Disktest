#include "opedb.h"
#include <QMessageBox>
#include <QDebug>

OpeDB::OpeDB(QObject *parent) : QObject(parent)
{
    m_db=QSqlDatabase::addDatabase("QSQLITE");

}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;
    return instance;
}

void OpeDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("D:\\study_my\\projectt\\myDisk\\TcpServer\\cloud.db");
    if (m_db.open()) {
        QSqlQuery query;
        query.exec("select * from usrInfo");
        while (query.next()) {
            QString data = QString("%1,%2,%3")
                                 .arg(query.value(0).toString())
                                 .arg(query.value(1).toString())
                                 .arg(query.value(2).toString());
             qDebug() << data;
        }
    }else {

        QMessageBox::critical(NULL,"open db","open fail");
    }
}

OpeDB::~OpeDB()
{
    m_db.close();
}

bool OpeDB::handleRegist(const char *name, const char *pwd)
{
//    char caQuery[123]={'\0'};
    if (NULL == name || NULL == pwd ) {
        qDebug()<<"name||pwd null";
        return  false;
    }
    QString data=QString("insert into usrInfo(name,pwd) values('%1','%2')").arg(name).arg(pwd);
    qDebug()<<data;
    QSqlQuery query;
    return query.exec(data);
}

bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    if (NULL == name || NULL == pwd ) {
        qDebug()<<"name||pwd null";
        return  false;
    }
    QString data=QString("select * from usrInfo where name = \'%1\' and pwd = \'%2\' and online=0").arg(name).arg(pwd);
    qDebug()<<data;
    QSqlQuery query;
    query.exec(data);
//    return  query.next();
    if (query.next()) {
        data=QString("update usrInfo set online=1 where name=\'%1\' and pwd=\'%2\'").arg(name).arg(pwd);
        qDebug()<<data;
        query.exec(data);

        return true;
    }else {
        return false;
    }


}

void OpeDB::handleOffine(const char *name)
{
    if (NULL == name) {
        qDebug()<<"name||pwd null";
        return ;
//        return  false;
    }
    QString data =QString("update usrInfo set online=0 where name=\'%1\'").arg(name);
    qDebug()<< data;
    QSqlQuery query;
    query.exec(data);

}

QStringList OpeDB::handleAllOnline()
{
    QString data =QString("select name from usrInfo where online=1");
    qDebug()<< data;
    QSqlQuery query;
    query.exec(data);
    QStringList res;
    res.clear();
    while (query.next()) {
        res.append(query.value(0).toString());
    }
    return res;

}

int OpeDB::handleSearchUsr(const char *name)
{
    if (NULL ==name) {
        return -1;
    }
    QString data =QString("select online from usrInfo where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
    if (query.next()) {
        int ret=query.value(0).toInt();
        if (1== ret) {
            return 1;
        }else if (0 ==ret) {
            return 0;
        }
    }else {
        return -1;
    }
}

int OpeDB::handleAddFriend(const char *pername, const char *name)
{
    if (NULL == pername || NULL == name ) {
        return -1;
    }
    if ( strcmp(pername,name)==0) {
        return -2;
    }
    QString data= QString("select * from friend where "
                          "(id=(select id from usrInfo where name = \'%1\' ) "
                          "and friendId= (select id from usrInfo where name =\'%2\'))"
                          "or "
                          "(id=(select id from usrInfo where name = \'%3\' )"
                          "and friendId= (select id from usrInfo where name =\'%4\'))")
            .arg(name).arg(pername).arg(pername).arg(name);
    qDebug()<<data;
    QSqlQuery query;
    query.exec(data);

    if (query.next()) {
        return 0;
    }else {
        QString data =QString("select online from usrInfo where name=\'%1\'").arg(pername);
        QSqlQuery query;
        query.exec(data);
        if (query.next()) {
            int ret=query.value(0).toInt();
            if (1== ret) {
                return 1;   //on
            }else if (0 ==ret) {
                return 2;       //off
            }
        }else {
            return 3; //no
        }
    }
}

int OpeDB::handleAddFriendAge(const char *pername, const char *name)
{
//    insert into friend (id, friendId) values(1,2)
//    INSERT INTO friend (id, friendId) SELECT a.id, b.id  FROM usrInfo a, usrInfo b WHERE a.name='A' AND b.name='B'
    QString data =QString("INSERT INTO friend (id, friendId)"
                          " SELECT a.id, b.id  "
                          "FROM usrInfo a, usrInfo b "
                          "WHERE a.name=\'%1\' AND b.name=\'%2\'").arg(pername).arg(name);
    QSqlQuery query;
//    query.exec(data);
    if (!query.exec(data)) {
//            qDebug() << "Add friend failed:" << query.lastError().text();
            return -1;
      }else {
        return 0;
    }
}

QStringList OpeDB::handleFlushFriend(const char *name)
{
    qDebug()<<"IN flush::db";
    QStringList strFriendList;
    strFriendList.clear();
    if (NULL == name) {
        return strFriendList;
    }
    QString data =QString("select name from usrInfo where online=1 "
                          "and "
                          "id in ("
                            "select id from friend where "
                                "friendId=(select id from usrInfo where name=\'%1\')"
                          ")"
                ).arg(name);
    QSqlQuery query;
    qDebug()<<data;
    query.exec(data);
    while (query.next()) {
        strFriendList.append(query.value(0).toString());
        qDebug()<<query.value(0).toString();
    }
    data =QString("select name from usrInfo where online=1 "
                          "and "
                          "id in ("
                            "select friendId from friend where "
                                "id=(select id from usrInfo where name=\'%1\')"
                          ")"
                ).arg(name);
    qDebug()<<data;
    query.exec(data);
    while (query.next()) {
        strFriendList.append(query.value(0).toString());
        qDebug()<<query.value(0).toString();
    }

    return strFriendList;
}

bool OpeDB::handleDelFriend(const char *name, const char *friendname)
{
    if (NULL==name || NULL==friendname) {
        return false;
    }
    QString data =QString("delete from friend where id=("
                          "select id from usrInfo where name=\'%1\') "
                          "and "
                          "friendId=("
                          "select id from usrInfo where name=\'%2\')"
                ).arg(name).arg(friendname);
    QSqlQuery query;
    qDebug()<<data;
    query.exec(data);
    data =QString("delete from friend where id=("
                          "select id from usrInfo where name=\'%1\') "
                          "and "
                          "friendId=("
                          "select id from usrInfo where name=\'%2\')"
                ).arg(friendname).arg(name);
    qDebug()<<data;
    query.exec(data);

    return true;
}
