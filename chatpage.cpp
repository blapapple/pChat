#include "chatpage.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QStyleOption>
#include <QUuid>

#include "chatitembase.h"
#include "picturebubble.h"
#include "tcpmgr.h"
#include "textbubble.h"
#include "ui_chatpage.h"
#include "usermgr.h"

ChatPage::ChatPage(QWidget *parent) : QWidget(parent), ui(new Ui::ChatPage) {
    ui->setupUi(this);
    // 设置按钮样式
    ui->receive_btn->SetState("normal", "hover", "press");
    ui->send_btn->SetState("normal", "hover", "press");
    ui->emo_label->SetState("normal", "hover", "press", "normal", "hover",
                            "press");
    ui->file_label->SetState("normal", "hover", "press", "normal", "hover",
                             "press");
    ui->chat_data_wid->setContentsMargins(0, 0, 0, 0);
    ui->chat_data_wid->setStyleSheet(
        "margin: 0px; padding: 0px; border: none;");  // 通过样式表覆盖
}

ChatPage::~ChatPage() { delete ui; }

void ChatPage::SetUserInfo(std::shared_ptr<UserInfo> user_info) {
    _user_info = user_info;
    // 设置ui界面
    ui->title_label->setText(_user_info->_name);
    ui->chat_data_list->removeAllItem();
    for (auto &msg : user_info->_chat_msgs) {
        AppendChatMsg(msg);
    }
}

void ChatPage::AppendChatMsg(std::shared_ptr<TextChatData> msg) {
    auto self_info = UserMgr::GetInstance()->GetUserInfo();
    ChatRole role;
    // todo... 添加聊天显示
    if (msg->_from_uid == self_info->_uid) {
        role = ChatRole::Self;
        ChatItembase *pChatItem = new ChatItembase(role);
        pChatItem->setUserName(self_info->_name);
        pChatItem->setUserIcon(QPixmap(self_info->_icon));
        QWidget *pBubble = nullptr;
        pBubble = new TextBubble(role, msg->_msg_content);
        pChatItem->setWidget(pBubble);
        ui->chat_data_list->appendChatItem(pChatItem);
    } else {
        role = ChatRole::Other;
        ChatItembase *pChatItem = new ChatItembase(role);
        auto friend_info =
            UserMgr::GetInstance()->GetFriendById(msg->_from_uid);
        if (friend_info == nullptr) {
            return;
        }
        pChatItem->setUserName(self_info->_name);
        pChatItem->setUserIcon(QPixmap(friend_info->_icon));
        QWidget *pBubble = nullptr;
        pBubble = new TextBubble(role, msg->_msg_content);
        pChatItem->setWidget(pBubble);
        ui->chat_data_list->appendChatItem(pChatItem);
    }
}

void ChatPage::paintEvent(QPaintEvent *event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatPage::on_send_btn_clicked() {
    if (_user_info == nullptr) {
        qDebug() << "friend_info is empty";
        return;
    }
    auto user_info = UserMgr::GetInstance()->GetUserInfo();

    auto pTextEdit = ui->chat_edit;
    ChatRole role = ChatRole::Self;
    QString userName = user_info->_name;
    QString userIcon = user_info->_icon;

    const QVector<MsgInfo> &msgList = pTextEdit->getMsgList();
    QJsonObject textObj;
    QJsonArray textArray;
    int txt_size = 0;

    for (int i = 0; i < msgList.size(); ++i) {
        if (msgList[i].content.length() > MAX_CHAT_TXT_LENGTH) {
            continue;
        }

        QString type = msgList[i].msgFlag;
        ChatItembase *pChatItem = new ChatItembase(role);
        pChatItem->setUserName(userName);
        pChatItem->setUserIcon(QPixmap(userIcon));
        QWidget *pBubble = nullptr;

        if (type == "text") {
            //  生成唯一id
            QUuid uuid = QUuid::createUuid();
            QString uuidString = uuid.toString();

            pBubble = new TextBubble(role, msgList[i].content);

            //  尽量将信息累积到一定数量再发给服务器
            if (txt_size + msgList[i].content.length() > MAX_CHAT_TXT_LENGTH) {
                textObj["fromuid"] = user_info->_uid;
                textObj["touid"] = _user_info->_uid;
                textObj["text_array"] = textArray;
                QJsonDocument doc(textArray);
                QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
                //  发送并清空之前累计的文本列表
                txt_size = 0;
                textArray = QJsonArray();
                textObj = QJsonObject();
                //  发送tcp请求给chat server，将信息存到服务器
                emit TcpMgr::GetInstance()->sig_send_data(
                    ReqId::ID_TEXT_CHAT_MSG_REQ, jsonData);
            }

            //  将bubble和uid绑定，以后可以等网络返回消息后设置是否送达
            //  _bubble_map[uuidString] = pBubble;
            txt_size += msgList[i].content.length();
            QJsonObject obj;
            QByteArray utf8Message = msgList[i].content.toUtf8();
            obj["content"] = QString::fromUtf8(utf8Message);
            obj["msgid"] = uuidString;
            textArray.append(obj);
            auto txt_msg = std::make_shared<TextChatData>(
                uuidString, obj["content"].toString(), user_info->_uid,
                _user_info->_uid);
            emit sig_append_send_chat_msg(txt_msg);
        } else if (type == "image") {
            pBubble = new PictureBubble(QPixmap(msgList[i].content), role);
        } else if (type == "file") {
        }

        if (pBubble != nullptr) {
            pChatItem->setWidget(pBubble);
            ui->chat_data_list->appendChatItem(pChatItem);
        }
    }

    qDebug() << "textArray is " << textArray;
    //  发送给服务器
    textObj["text_array"] = textArray;
    textObj["fromuid"] = user_info->_uid;
    textObj["touid"] = _user_info->_uid;
    QJsonDocument doc(textObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    //  发送并清空之前累积的文本列表
    txt_size = 0;
    textArray = QJsonArray();
    textObj = QJsonObject();
    //  发送tcp请求给chat server
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ,
                                              jsonData);
}
