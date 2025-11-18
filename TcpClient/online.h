#ifndef ONLINE_H
#define ONLINE_H

#include <QWidget>
#include "protocol.h"
namespace Ui {
class Online;
}

class Online : public QWidget
{
    Q_OBJECT

public:
    explicit Online(QWidget *parent = nullptr);
    ~Online();
    static Online &getInstance();
    void showUsr(PDU *pdu);
     Ui::Online* getUI();
private slots:
    void on_addFriend_pb_clicked();

private:
    Ui::Online *ui;
};

#endif // ONLINE_H
