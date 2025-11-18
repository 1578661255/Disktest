#include "tcpclient.h"
#include <QApplication>
#include"sharefile.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFont font;
    font.setFamily("Microsoft YaHei"); // Windows
    font.setPointSize(20);
    a.setFont(font);

    TcpClient &w = TcpClient::getInstance();
    w.setWindowTitle("客户端");
    w.show();
    return a.exec();
}
