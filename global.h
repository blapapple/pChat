#ifndef GLOBAL_H
#define GLOBAL_H
#include <QWidget>
#include <functional>
#include "QStyle"
#include <memory>
#include <iostream>
#include <mutex>
#include <QRegularExpression>
#include <QByteArray>
#include <QNetworkReply>
#include <QDir>
#include <QSettings>
#include <QPainter>
/**
 * @brief repolish 用来刷新qss
 */
extern std::function<void(QWidget*)> repolish;
extern std::function<QString(QString)> xorString;

enum ReqId{
    ID_GET_VARIFY_CODE = 1001,  //获取验证码
    ID_REG_USER = 1002,         //注册用户
    ID_RESET_PWD = 1003,        //重置密码
    ID_LOGIN_USER = 1004,       //用户登录
    ID_CHAT_LOGIN = 1005,       //登录聊天服务器
    ID_CHAT_LOGIN_RSP = 1006,   //登录聊天服务器回包
};

enum Modules{
    REGISTERMOD = 0,    //注册模块
    RESETMOD = 1,
    LOGINMOD = 2,
};

enum ErrorCodes{
    SUCCESS = 0,
    ERR_JSON = 1,       //json 解析失败
    ERR_NETWORK = 2,    //网络错误
};

enum TipErr{
    TIP_SUCCESS = 0,
    TIP_EMAIL_ERR = 1,     //邮箱错误
    TIP_PWD_ERR = 2,        //密码错误
    TIP_CONFIRM_ERR = 3,    //确认密码错误
    TIP_PWD_CONFIRM = 4,    //密码和确认密码不匹配
    TIP_VARIFY_ERR = 5,     //验证码不匹配
    TIP_USER_ERR = 6        //用户错误
};

enum ClickLbState{
    Normal = 0,
    Selected = 1
};

enum ListItemType{
    CHAT_USER_ITEM,         //聊天用户
    CONTACT_USER_ITEM,      //联系人用户
    SEARCH_USER_ITEM,       //搜索到的用户
    ADD_USER_TIP_ITEM,      //提示添加用户
    INVALID_item,           //不可点击条目
    GROUP_TIP_ITEM,         //分组提示条目
};

struct ServerInfo
{
    QString Host;
    QString Port;
    QString Token;
    int Uid;
};

enum ChatUIMode{
    SearchMode,
    ChatMode,
    ContactMode,
};

enum ChatRole{
    Self,
    Other
};

struct MsgInfo{
    QString msgFlag;    //text, image, file
    QString content;    //表示文件和图象的url, 文本信息
    QPixmap pixmap;     //文件和图片的缩略图
};

extern QString gate_url_prefix;

#endif // GLOBAL_H
