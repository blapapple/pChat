#include "chatpage.h"

#include <QPainter>
#include <QStyleOption>

#include "chatitembase.h"
#include "picturebubble.h"
#include "textbubble.h"
#include "ui_chatpage.h"

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

void ChatPage::paintEvent(QPaintEvent *event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatPage::on_send_btn_clicked() {
    auto pTextEdit = ui->chat_edit;
    ChatRole role = ChatRole::Self;
    QString userName = QStringLiteral("潘金金");
    QString userIcon = ":/resources/images/20240617101735.jpg";

    const QVector<MsgInfo> &msgList = pTextEdit->getMsgList();
    for (int i = 0; i < msgList.size(); ++i) {
        QString type = msgList[i].msgFlag;
        ChatItembase *pChatItem = new ChatItembase(role);
        pChatItem->setUserName(userName);
        pChatItem->setUserIcon(QPixmap(userIcon));

        QWidget *pBubble = nullptr;
        if (type == "text") {
            pBubble = new TextBubble(role, msgList[i].content);
        } else if (type == "image") {
            pBubble = new PictureBubble(QPixmap(msgList[i].content), role);
        } else if (type == "file") {
        }

        if (pBubble != nullptr) {
            pChatItem->setWidget(pBubble);
            ui->chat_data_list->appendChatItem(pChatItem);
        }
    }
}
