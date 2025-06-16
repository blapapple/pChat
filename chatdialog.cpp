#include "chatdialog.h"

#include <QAction>
#include <QListWidget>
#include <QListWidgetItem>
#include <QRandomGenerator>
#include <QWidget>

#include "chatuserwid.h"
#include "conuseritem.h"
#include "listitembase.h"
#include "loadingdlg.h"
#include "tcpmgr.h"
#include "ui_chatdialog.h"
#include "usermgr.h"

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ChatDialog),
      _mode(ChatUIMode::ChatMode),
      _state(ChatUIMode::ChatMode),
      _b_loading(false),
      _cur_chat_uid(0),
      _last_widget(nullptr) {
    ui->setupUi(this);
    ui->add_btn->SetState("normal", "hover", "press");
    ui->search_edit->setMaxLength(30);
    // 强制消除边距
    ui->chat_user_wd->setContentsMargins(0, 0, 0, 0);
    ui->chat_user_wd->setStyleSheet("margin: 0px; padding: 0px; border: none;");

    // 创建一个搜索动作和icon到line头部
    QAction *searchAction = new QAction(ui->search_edit);
    searchAction->setIcon(QIcon(":/resources/images/search.png"));
    ui->search_edit->addAction(searchAction, QLineEdit::LeadingPosition);
    ui->search_edit->setPlaceholderText(QStringLiteral("搜索"));

    // 创建一个清除动作并设置图标
    QAction *clearAction = new QAction(ui->search_edit);
    clearAction->setIcon(QIcon(":/resources/images/close_transparent.png"));
    // 初始不显示清除图标，将清除动作添加到lineEdit末尾位置
    ui->search_edit->addAction(clearAction, QLineEdit::TrailingPosition);
    // 需要显示清除图标时，更改为实际的清除图标
    connect(ui->search_edit, &QLineEdit::textChanged,
            [clearAction](const QString &text) {
                if (!text.isEmpty()) {
                    clearAction->setIcon(
                        QIcon(":/resources/images/close_search.png"));
                } else {
                    clearAction->setIcon(
                        QIcon(":/resources/images/close_transparent.png"));
                }
            });

    // 清除文本
    connect(clearAction, &QAction::triggered, [this, clearAction]() {
        ui->search_edit->clear();
        clearAction->setIcon(QIcon(":/resources/images/close_transparent.png"));
        ui->search_edit->clearFocus();
        ShowSearch(false);
    });

    ShowSearch(false);

    connect(ui->chat_user_list, &ChatUserList::sig_loading_chat_user, this,
            &ChatDialog::slot_loading_chat_user);
    addChatUserList();

    QPixmap pixmap(":/resources/images/20240617101735.jpg");
    ui->side_head_label->setPixmap(pixmap);  // 将图片设置到QLabel上
    QPixmap scaledPixmap =
        pixmap.scaled(ui->side_head_label->size(),
                      Qt::KeepAspectRatio);  // 将图片缩放到label的大小
    ui->side_head_label->setPixmap(
        scaledPixmap);  // 将缩放后的图片设置到QLabel上
    ui->side_head_label->setScaledContents(
        true);  // 设置QLabel自动缩放图片内容以适应大小

    ui->side_chat_label->setProperty("state", "normal");
    ui->side_chat_label->SetState("normal", "hover", "pressed",
                                  "selected_normal", "selected_hover",
                                  "selected_pressed");
    ui->side_contact_label->SetState("normal", "hover", "pressed",
                                     "selected_normal", "selected_hover",
                                     "selected_pressed");

    AddLBGroup(ui->side_chat_label);
    AddLBGroup(ui->side_contact_label);
    connect(ui->side_chat_label, &StateWidget::clicked, this,
            &ChatDialog::slot_side_chat);
    connect(ui->side_contact_label, &StateWidget::clicked, this,
            &ChatDialog::slot_side_contact);

    connect(ui->search_edit, &QLineEdit::textChanged, this,
            &ChatDialog::slot_text_changed);
    // 检测鼠标点击位置判断是否清空搜索框
    this->installEventFilter(this);
    ui->side_chat_label->SetSelected(true);

    // 为searchlist设置searchedit
    ui->search_list->SetSearchEdit(ui->search_edit);

    //  连接申请添加好友信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_friend_apply, this,
            &ChatDialog::slot_apply_friend);

    //  连接认证添加好友信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_add_auth_friend, this,
            &ChatDialog::slot_add_auth_friend);

    //  链接好友认证结果回复信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_rsp, this,
            &ChatDialog::slot_auth_rsp);

    // 连接searchlist跳转聊天信号
    connect(ui->search_list, &SearchList::sig_jump_chat_item, this,
            &ChatDialog::slot_jump_chat_item);

    //  连接加载联系人的信号和槽函数
    connect(ui->con_user_list, &ContactUserList::sig_loading_contact_user, this,
            &ChatDialog::slot_loading_contact_user);

    //  点击联系人item后跳转到用户信息展示槽函数
    connect(ui->con_user_list, &ContactUserList::sig_switch_friend_info_page,
            this, &ChatDialog::slot_friend_info_page);

    //  连接联系人页面点击好友申请条目的信号
    connect(ui->con_user_list, &ContactUserList::sig_switch_apply_friend_page,
            this, &ChatDialog::slot_switch_apply_friend_page);

    //  好友信息界面点击跳转到聊天界面
    connect(ui->friend_info_page, &FriendInfoPage::sig_jump_chat_item, this,
            ChatDialog::slot_jump_chat_item_from_infopage);

    //  设置中心部件为chatpage
    ui->stackedWidget->setCurrentWidget(ui->chat_page);

    //  聊天列表点击跳转聊天界面
    connect(ui->chat_user_list, &QListWidget::itemClicked, this,
            &ChatDialog::slot_item_clicked);

    //  发送信息缓存到本地
    connect(ui->chat_page, &ChatPage::sig_append_send_chat_msg, this,
            &ChatDialog::slot_append_send_chat_msg);

    //  聊天信息到达
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_text_chat_msg, this,
            &ChatDialog::slot_text_chat_msg);
}

ChatDialog::~ChatDialog() { delete ui; }

void ChatDialog::addChatUserList() {
    // 先按照好友列表加载聊天记录，等以后客户端实现聊天记录数据库之后再按照最后信息排序
    auto friend_list = UserMgr::GetInstance()->GetChatListPerPage();
    if (friend_list.empty() == false) {
        for (auto &friend_ele : friend_list) {
            auto find_iter = _chat_items_added.find(friend_ele->_uid);
            if (find_iter != _chat_items_added.end()) {
                continue;
            }
            auto *chat_user_wid = new ChatUserWid();
            auto user_info = std::make_shared<UserInfo>(friend_ele);
            chat_user_wid->SetInfo(user_info);
            QListWidgetItem *item = new QListWidgetItem;
            // qDebug()<<"chat_user_wid sizeHint is " <<
            // chat_user_wid->sizeHint();
            item->setSizeHint(chat_user_wid->sizeHint());
            ui->chat_user_list->addItem(item);
            ui->chat_user_list->setItemWidget(item, chat_user_wid);
            _chat_items_added.insert(friend_ele->_uid, item);
        }

        // 更新已加载条目
        UserMgr::GetInstance()->UpdateChatLoadedCount();
    }

    // 创建item
    for (int i = 0; i < 25; i++) {
        int randomValue =
            QRandomGenerator::global()->bounded(100);  // 生成0-99随机整数
        int str_i = randomValue % strs.size();
        int head_i = randomValue % heads.size();
        int name_i = randomValue % names.size();

        auto *chat_user_wid = new ChatUserWid();
        auto user_info = std::make_shared<UserInfo>(
            i, names[name_i], names[name_i], heads[head_i], 0);
        chat_user_wid->SetInfo(user_info);
        QListWidgetItem *item = new QListWidgetItem;
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, chat_user_wid);
    }
}

void ChatDialog::ClearLabelState(StateWidget *lb) {
    for (auto &ele : _lb_list) {
        if (ele == lb) {
            continue;
        }
        ele->ClearState();
    }
}

void ChatDialog::SetSelectChatItem(int uid) {
    if (ui->chat_user_list->count() <= 0) {
        return;
    }

    if (uid == 0) {
        ui->chat_user_list->setCurrentRow(0);
        QListWidgetItem *firstItem = ui->chat_user_list->item(0);
        if (!firstItem) {
            return;
        }

        // 转为widget
        QWidget *widget = ui->chat_user_list->itemWidget(firstItem);
        if (!widget) {
            return;
        }

        auto con_item = qobject_cast<ChatUserWid *>(widget);
        if (!con_item) {
            return;
        }
        _cur_chat_uid = con_item->GetUserInfo()->_uid;
        return;
    }

    auto find_iter = _chat_items_added.find(uid);
    if (find_iter == _chat_items_added.end()) {
        qDebug() << "SetSelectChatItem not found uid " << uid;
        ui->chat_user_list->setCurrentRow(0);
        return;
    }

    ui->chat_user_list->setCurrentItem(find_iter.value());
    _cur_chat_uid = uid;
}

void ChatDialog::SetSelectChatPage(int uid) {
    if (ui->chat_user_list->count() <= 0) {
        return;
    }

    if (uid == 0) {
        auto item = ui->chat_user_list->item(0);

        // 转为widget
        QWidget *widget = ui->chat_user_list->itemWidget(item);
        if (!widget) {
            return;
        }

        auto con_item = qobject_cast<ChatUserWid *>(widget);
        if (!con_item) {
            return;
        }
        auto user_info = con_item->GetUserInfo();
        ui->chat_page->SetUserInfo(user_info);
        return;
    }

    auto find_iter = _chat_items_added.find(uid);
    if (find_iter == _chat_items_added.end()) {
        return;
    }

    //  转为widget
    QWidget *widget = ui->chat_user_list->itemWidget(find_iter.value());
    if (!widget) {
        return;
    }

    //  判断转化为自定义的widget
    //  对自定义widget进行操作，将item转化为基类listitembase
    ListItemBase *customItem = qobject_cast<ListItemBase *>(widget);
    if (!customItem) {
        qDebug() << "qobject_cast<ListItemBase*>(widget) is nullptr";
        return;
    }

    auto itemType = customItem->GetItemType();
    if (itemType == CHAT_USER_ITEM) {
        auto con_item = qobject_cast<ChatUserWid *>(customItem);
        if (!con_item) {
            return;
        }

        //  设置信息
        auto user_info = con_item->GetUserInfo();
        ui->chat_page->SetUserInfo(user_info);
        return;
    }
}

bool ChatDialog::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        handleGlobalMousePress(mouseEvent);
    }
    return QDialog::eventFilter(watched, event);
}

void ChatDialog::handleGlobalMousePress(QMouseEvent *event) {
    // 先判断是否处于搜索模式，如果不处于则不用管
    if (_mode != ChatUIMode::SearchMode) {
        return;
    }

    // 将鼠标点击位置转换为搜索列表坐标系中的位置
    QPoint posInSearchList =
        ui->search_list->mapFromGlobal(event->globalPosition().toPoint());
    if (!ui->search_list->rect().contains(posInSearchList)) {
        ui->search_list->clear();
        ShowSearch(false);
    }
}

void ChatDialog::ShowSearch(bool bsearch) {
    if (bsearch) {
        ui->chat_user_list->hide();
        ui->con_user_list->hide();
        ui->search_list->show();
        _mode = ChatUIMode::SearchMode;
    } else if (_state == ChatUIMode::ChatMode) {
        ui->chat_user_list->show();
        ui->con_user_list->hide();
        ui->search_list->hide();
        _mode = ChatUIMode::ChatMode;
    } else if (_state == ChatUIMode::ContactMode) {
        ui->chat_user_list->hide();
        ui->con_user_list->show();
        ui->search_list->hide();
        _mode = ChatUIMode::ContactMode;
    }
}

void ChatDialog::AddLBGroup(StateWidget *lb) { _lb_list.push_back(lb); }

void ChatDialog::loadMoreChatUser() {
    auto friend_list = UserMgr::GetInstance()->GetChatListPerPage();
    if (friend_list.empty() == false) {
        for (auto &friend_ele :
             friend_list) {  // 从好友数据再往后取出一定数量好友添加进列表中
            auto find_iter = _chat_items_added.find(friend_ele->_uid);
            if (find_iter != _chat_items_added.end()) {
                continue;
            }
            auto *chat_user_wid = new ChatUserWid();
            auto user_info = std::make_shared<UserInfo>(friend_ele);
            chat_user_wid->SetInfo(user_info);
            QListWidgetItem *item = new QListWidgetItem;
            // qDebug()<<"chat_user_wid sizeHint is " <<
            // chat_user_wid->sizeHint();
            item->setSizeHint(chat_user_wid->sizeHint());
            ui->chat_user_list->addItem(item);
            ui->chat_user_list->setItemWidget(item, chat_user_wid);
            _chat_items_added.insert(friend_ele->_uid, item);
        }

        // 更新已加载条目
        UserMgr::GetInstance()->UpdateChatLoadedCount();
    }
}

void ChatDialog::loadMoreConUser() {
    auto friend_list = UserMgr::GetInstance()->GetConListPerPage();
    if (friend_list.empty() == false) {
        for (auto &friend_ele : friend_list) {
            auto *chat_user_wid = new ConUserItem();
            chat_user_wid->SetInfo(friend_ele->_uid, friend_ele->_name,
                                   friend_ele->_icon);
            QListWidgetItem *item = new QListWidgetItem;
            // qDebug()<<"chat_user_wid sizeHint is " <<
            // chat_user_wid->sizeHint();
            item->setSizeHint(chat_user_wid->sizeHint());
            ui->con_user_list->addItem(item);
            ui->con_user_list->setItemWidget(item, chat_user_wid);
        }

        // 更新已加载条目
        UserMgr::GetInstance()->UpdateContactLoadedCount();
    }
}

void ChatDialog::UpdateChatMsg(
    std::vector<std::shared_ptr<TextChatData>> msgdata) {
    for (auto &msg : msgdata) {
        if (msg->_from_uid != _cur_chat_uid) {
            break;
        }
        ui->chat_page->AppendChatMsg(msg);
    }
}

void ChatDialog::slot_loading_chat_user() {
    if (_b_loading) {
        return;
    }

    _b_loading = true;
    LoadingDlg *loadingDialog = new LoadingDlg(this);
    loadingDialog->setModal(true);
    loadingDialog->show();
    qDebug() << "add new data to list ...";
    loadMoreChatUser();
    // 加载完成后关闭对话框
    loadingDialog->deleteLater();

    _b_loading = false;
}

void ChatDialog::slot_side_chat() {
    qDebug("receive side chat clicked");
    ClearLabelState(ui->side_chat_label);
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    _state = ChatUIMode::ChatMode;
    ShowSearch(false);
}

void ChatDialog::slot_side_contact() {
    qDebug("receive side contact clicked");
    ClearLabelState(ui->side_contact_label);
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
    _state = ChatUIMode::ContactMode;
    ShowSearch(false);
}

void ChatDialog::slot_text_changed(const QString &str) {
    if (!str.isEmpty()) {
        ShowSearch(true);
    } else {
        ShowSearch(false);
    }
}

void ChatDialog::slot_loading_contact_user() {
    qDebug() << "slot loading contact user";
    if (_b_loading) {
        return;
    }
    _b_loading = true;
    LoadingDlg *loadingDialog = new LoadingDlg(this);
    loadingDialog->setModal(true);
    loadingDialog->show();
    qDebug() << "add new data to list......";
    loadMoreConUser();
    // 加载完成后关闭对话框
    loadingDialog->deleteLater();
    _b_loading = false;
}

void ChatDialog::slot_apply_friend(std::shared_ptr<AddFriendApply> apply) {
    qDebug() << "receive apply friend slot, applyuid is " << apply->_from_uid
             << " name is " << apply->_name << " desc is " << apply->_desc;

    bool b_already = UserMgr::GetInstance()->AlreadyApply(apply->_from_uid);
    if (b_already) {  // 如果已经有申请了
        return;
    }

    UserMgr::GetInstance()->addOrUpdateApply(
        std::make_shared<ApplyInfo>(apply));
    ui->side_contact_label->ShowReadPoint(true);
    ui->con_user_list->ShowRedPoint(true);
    ui->friend_apply_page->AddNewApply(apply);
}

void ChatDialog::slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info) {
    auto bfriend = UserMgr::GetInstance()->CheckFriendById(auth_info->_uid);
    if (bfriend) {
        return;
    }

    UserMgr::GetInstance()->AddFriend(auth_info);
    int randomValue = QRandomGenerator::global()->bounded(100);
    int str_i = randomValue % strs.size();
    int head_i = randomValue % heads.size();
    int name_i = randomValue % names.size();

    // 创建一个聊天联系框，然后添加进list中
    auto *chat_user_wid = new ChatUserWid();
    auto user_info = std::make_shared<UserInfo>(auth_info);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(auth_info->_uid, item);
}

void ChatDialog::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp) {
    qDebug() << "receive slot_auth_rsp uid is " << auth_rsp->_uid << " name is "
             << auth_rsp->_name << " nick is " << auth_rsp->_nick;
    auto bfriend = UserMgr::GetInstance()->CheckFriendById(auth_rsp->_uid);
    if (bfriend) {
        return;
    }

    UserMgr::GetInstance()->AddFriend(auth_rsp);
    int randomValue = QRandomGenerator::global()->bounded(100);
    int str_i = randomValue % strs.size();
    int head_i = randomValue % heads.size();
    int name_i = randomValue % names.size();

    // 创建一个聊天联系框，然后添加进list中
    auto *chat_user_wid = new ChatUserWid();
    auto user_info = std::make_shared<UserInfo>(auth_rsp);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(auth_rsp->_uid, item);
}

void ChatDialog::slot_jump_chat_item(std::shared_ptr<SearchInfo> si) {
    qDebug() << "slot jump chat item";
    auto find_iter = _chat_items_added.find(si->_uid);
    if (find_iter != _chat_items_added.end()) {
        qDebug() << "jump to chat item , uid is " << si->_uid;
        ui->chat_user_list->scrollToItem(find_iter.value());
        ui->side_chat_label->SetSelected(true);
        SetSelectChatItem(si->_uid);
        // 更新聊天界面信息
        SetSelectChatPage(si->_uid);
        slot_side_chat();
        return;
    }
    // 如果没找到，则创建新的插入listwidget
    auto *chat_user_wid = new ChatUserWid();
    auto user_info = std::make_shared<UserInfo>(si);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);

    _chat_items_added.insert(si->_uid, item);

    ui->side_chat_label->SetSelected(true);
    SetSelectChatItem(si->_uid);
    // 更新聊天界面信息
    SetSelectChatPage(si->_uid);
    slot_side_chat();
}

void ChatDialog::slot_friend_info_page(std::shared_ptr<FriendInfo> user_info) {
    qDebug() << "receive switch friend info page sig";
    _last_widget = ui->friend_info_page;
    ui->stackedWidget->setCurrentWidget(ui->friend_info_page);
    ui->friend_info_page->SetInfo(user_info);
}

void ChatDialog::slot_switch_apply_friend_page() {
    qDebug() << "receive switch apply friend page sig";
    _last_widget = ui->friend_apply_page;
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
}

void ChatDialog::slot_jump_chat_item_from_infopage(
    std::shared_ptr<FriendInfo> user_info) {
    qDebug() << "slot jump chat item";
    auto find_iter = _chat_items_added.find(user_info->_uid);
    if (find_iter != _chat_items_added.end()) {
        qDebug() << "jump to chat item , uid is " << user_info->_uid;
        ui->chat_user_list->scrollToItem(find_iter.value());
        ui->side_chat_label->SetSelected(true);
        SetSelectChatItem(user_info->_uid);
        // 更新聊天界面信息
        SetSelectChatPage(user_info->_uid);
        slot_side_chat();
        return;
    }
    // 如果没找到，则创建新的插入listwidget
    auto *chat_user_wid = new ChatUserWid();
    chat_user_wid->SetInfo(std::make_shared<UserInfo>(user_info));
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);

    _chat_items_added.insert(user_info->_uid, item);

    ui->side_chat_label->SetSelected(true);
    SetSelectChatItem(user_info->_uid);
    // 更新聊天界面信息
    SetSelectChatPage(user_info->_uid);
    slot_side_chat();
}

void ChatDialog::slot_item_clicked(QListWidgetItem *item) {
    QWidget *widget =
        ui->chat_user_list->itemWidget(item);  // 获取自定义widget对象
    if (!widget) {
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }
    // 对自定义widget进行操作， 将item 转化为基类ListItemBase
    ListItemBase *customItem = qobject_cast<ListItemBase *>(widget);
    if (!customItem) {
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }
    auto itemType = customItem->GetItemType();
    if (itemType == ListItemType::INVALID_ITEM ||
        itemType == ListItemType::GROUP_TIP_ITEM) {
        qDebug() << "slot invalid item clicked ";
        return;
    }
    if (itemType == ListItemType::CHAT_USER_ITEM) {
        // 创建对话框，提示用户
        qDebug() << "chat user item clicked ";
        auto chat_item = qobject_cast<ChatUserWid *>(customItem);
        auto user_info = chat_item->GetUserInfo();
        // 跳转到聊天界面
        ui->chat_page->SetUserInfo(user_info);
        _cur_chat_uid = user_info->_uid;
        return;
    }
}

void ChatDialog::slot_append_send_chat_msg(
    std::shared_ptr<TextChatData> msgdata) {
    if (_cur_chat_uid == 0) {
        return;
    }
    auto find_iter = _chat_items_added.find(_cur_chat_uid);
    if (find_iter == _chat_items_added.end()) {
        return;
    }

    // 转为widget
    QWidget *widget = ui->chat_user_list->itemWidget(find_iter.value());
    if (widget == nullptr) {
        return;
    }

    ListItemBase *customItem = qobject_cast<ListItemBase *>(widget);
    if (!customItem) {
        qDebug() << "qobject_cast<ListItemBase*>(widget) is nullptr";
        return;
    }

    auto itemType = customItem->GetItemType();
    if (itemType == CHAT_USER_ITEM) {
        auto con_item = qobject_cast<ChatUserWid *>(customItem);
        if (!con_item) {
            return;
        }

        //  确保每一个wid都有历史聊天记录
        auto user_info = con_item->GetUserInfo();
        user_info->_chat_msgs.emplace_back(msgdata);
        //  下面这个逻辑之后再理解一下
        std::vector<std::shared_ptr<TextChatData>> msg_vec;
        msg_vec.emplace_back(msgdata);
        UserMgr::GetInstance()->AppendFriendChatMsg(_cur_chat_uid, msg_vec);
        return;
    }
}

void ChatDialog::slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg) {
    auto find_iter = _chat_items_added.find(msg->_from_uid);
    if (find_iter != _chat_items_added.end()) {
        qDebug() << "set chat item msg, uid is " << msg->_from_uid;
        QWidget *widget = ui->chat_user_list->itemWidget(find_iter.value());
        auto chat_wid = qobject_cast<ChatUserWid *>(widget);
        if (!chat_wid) {
            return;
        }
        chat_wid->updateLastMsg(msg->_chat_msgs);
        //  更新当前聊天页面记录
        UpdateChatMsg(msg->_chat_msgs);
        UserMgr::GetInstance()->AppendFriendChatMsg(msg->_from_uid,
                                                    msg->_chat_msgs);
        return;
    }

    // 如果没找到，则创建新的插入listwidget
    auto *chat_user_wid = new ChatUserWid();
    // 查询好友信息
    auto fi_ptr = UserMgr::GetInstance()->GetFriendById(msg->_from_uid);
    chat_user_wid->SetInfo(std::make_shared<UserInfo>(fi_ptr));
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(chat_user_wid->sizeHint());
    chat_user_wid->updateLastMsg(msg->_chat_msgs);
    UserMgr::GetInstance()->AppendFriendChatMsg(msg->_from_uid,
                                                msg->_chat_msgs);
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(msg->_from_uid, item);
}
