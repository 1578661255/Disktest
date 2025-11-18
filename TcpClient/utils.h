#ifndef UTILS_H
#define UTILS_H
#include <QString>

class utils
{
public:
    utils();
    /**

     * @brief 对输入字符串进行 SHA-256 哈希，并返回十六进制字符串（小写）

     * @param input 明文字符串（如密码）

     * @return 64 字节的十六进制哈希值，例如 "a3f5...c8d2"

     */

    static QString sha256(const QString &input);
};

#endif // UTILS_H
