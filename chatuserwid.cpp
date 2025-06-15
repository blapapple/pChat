#include "chatuserwid.h"

#include "ui_chatuserwid.h"

ChatUserWid::ChatUserWid(QWidget *parent)
    : ListItemBase(parent), ui(new Ui::ChatUserWid) {
    ui->setupUi(this);
    SetItemType(ListItemType::CHAT_USER_ITEM);
}

ChatUserWid::~ChatUserWid() { delete ui; }

QSize ChatUserWid::sizeHint() const { return QSize(200, 55); }

void ChatUserWid::SetInfo(std::shared_ptr<UserInfo> user_info) {
    _user_info = user_info;

    QPixmap pixmap(_user_info->_icon);

    // 设置图片自动缩放
    ui->icon_label->setPixmap(pixmap.scaled(
        ui->icon_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_label->setScaledContents(true);

    ui->user_name_label->setText(_user_info->_name);
    ui->user_chat_label->setText(_user_info->_last_msg);
    ui->time_label->setText(QTime::currentTime().toString("hh:mm"));
}

std::shared_ptr<UserInfo> ChatUserWid::GetUserInfo() const {
    return _user_info;
}
