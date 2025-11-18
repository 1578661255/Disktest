#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef unsigned int uint;

#define REGIST_OK "regist ok"
#define REGIST_FAILED "regist failed:name existed"

#define LOGIN_OK "login ok"
#define LOGIN_FAILED "login failed:name error o pwd error o relogin"

#define SEARCH_USR_NO "no such people"
#define SEARCH_USR_ONLINE "online"
#define SEARCH_USR_OFFLINE "offline"

#define UNKNOW_ERROR "unknow error"
#define EXISTED_FRIEND "friend exist"
#define ADD_FRIEND_OFFLINE "usr offline"
#define ADD_FRIEND_NO_EXIST "usr not EXIST"
#define ADD_FRIEND_SELF "cannot addSelf"

#define DEL_FRIEND_OK "del friend ok"

#define DIR_NO_EXIST "cur dir not exist"
#define FILE_NAME_EXIST "file name exist"
#define CREATE_DIR_OK "create dir ok"

#define DEL_DIR_OK "delete dir ok"
#define DEL_DIR_FAILURED "delete dir failured: is regular file"

#define RENAME_FILE_OK "rename file ok"
#define RENAME_FILE_FAILURED "rename file failured"

#define ENTER_DIR_FAILURED "enter dir failured: is reg file"
#define ENTER_DIR_OK "enter dir ok"

#define UPLOAD_FILE_OK "upload file ok"
#define UPLOAD_FILE_FAILURED "upload file failured"


#define DEL_FILE_OK "delete file ok"
#define DEL_FILE_FAILURED "delete file failured: is dir"

#define SHARE_FILE_OK "share file ok"

#define MOVE_FILE_OK "move file ok"
#define MOVE_FILE_FAILURED "move file failured:: is reg file"

#define COMMON_ERR  "operator failed:system  is busy"
//#define ENTER
enum ENUM_MSG_TYPE {
    ENUM_MSG_TYPE_MIN=0,
    ENUM_MSG_TYPE_REGIST_REQUEST,
    ENUM_MSG_TYPE_REGIST_RESPOND,
    ENUM_MSG_TYPE_LOGIN_REQUEST,
    ENUM_MSG_TYPE_LOGIN_RESPOND,
    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,
    ENUM_MSG_TYPE_SEARCH_USR_REQUEST,
    ENUM_MSG_TYPE_SEARCH_USR_RESPOND,
    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND,

    ENUM_MSG_TYPE_ADD_FRIEND_AGGREE,
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE,

    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST,
    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND,
    ENUM_MSG_TYPE_DEL_FRIEND_REQUEST,
    ENUM_MSG_TYPE_DEL_FRIEND_RESPOND,

    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST,
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND,

    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,
    ENUM_MSG_TYPE_GROUP_CHAT_RESPOND,

    ENUM_MSG_TYPE_CREATE_DIR_REQUEST,
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND,

    ENUM_MSG_TYPE_FLUSH_FILE_REQUEST,
    ENUM_MSG_TYPE_FLUSH_FILE_RESPOND,

    ENUM_MSG_TYPE_DEL_DIR_REQUEST,
    ENUM_MSG_TYPE_DEL_DIR_RESPOND,


    ENUM_MSG_TYPE_RENAME_FILE_REQUEST,
    ENUM_MSG_TYPE_RENAME_FILE_RESPOND,

    ENUM_MSG_TYPE_ENTER_FILE_REQUEST,
    ENUM_MSG_TYPE_ENTER_FILE_RESPOND,

    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST,
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND,

    ENUM_MSG_TYPE_DEL_FILE_REQUEST,
    ENUM_MSG_TYPE_DEL_FILE_RESPOND,

    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST,
    ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND,

    ENUM_MSG_TYPE_SHARE_FILE_REQUEST,//37
    ENUM_MSG_TYPE_SHARE_FILE_RESPOND,
    ENUM_MSG_TYPE_SHARE_FILE_NOTE,
    ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND,

    ENUM_MSG_TYPE_MOVE_FILE_REQUEST,//41
    ENUM_MSG_TYPE_MOVE_FILE_RESPOND,
    //    ENUM_MSG_TYPE_REQUEST,
    //    ENUM_MSG_TYPE_RESPOND,
//    ENUM_MSG_TYPE_REQUEST,
//    ENUM_MSG_TYPE_RESPOND,
    ENUM_MSG_TYPE_MAX=0x00ffffff,

};

struct FileInfo {
    char caName[32];
    int iFileType;

};

struct PDU {
    uint uiPDULen;
    uint uiMsgType;
    char caData[64];
    uint uiMsgLen;
    int caMsg[];
};

PDU *mkPDU(uint uiMsgLen);





#endif // PROTOCOL_H
