#include "tcpclient.h"
//#include "opewidget.h"
//#include "online.h"
//#include "friend.h"
//#include "book.h"
#include <QApplication>
//#include <QMessageBox>
#include "sharefile.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont font("Times",24,QFont::Bold);
    a.setFont(font);

//    ShareFile w;
//    w.test();
//    TcpClient w;
//    w.show();
//    Online w;
//    w.show();
//    Friend w;
//    w.show();
//    OpeWidget w;
    TcpClient::getInstance().show();
//    QMessageBox::information(NULL,"添加好友",QString("%1 want add u as friend?").arg("caName"),
//                             QMessageBox::Yes,QMessageBox::No);
//    Book w;

//    w.show();
    return a.exec();
}
