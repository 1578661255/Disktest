#include "utils.h"
#include <QCryptographicHash>


utils::utils()
{

}

QString utils::sha256(const QString &input)

{

    QByteArray hash = QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex(); // 返回小写十六进制字符串，长度固定为 64
}
