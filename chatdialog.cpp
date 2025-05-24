#include "chatdialog.h"

#include <QAction>
#include <QRandomGenerator>

#include "chatuserwid.h"
#include "listitembase.h"
#include "loadingdlg.h"
#include "ui_chatdialog.h"

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ChatDialog),
      _mode(ChatUIMode::ChatMode),
      _state(ChatUIMode::ChatMode),
      _b_loading(false) {
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
}

ChatDialog::~ChatDialog() { delete ui; }

// 作测试用数据

std::vector<QString> strs = {
    "hello world !",
    "nihao",
    "你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好",
    "manba out",
    "what can i say",
    "KFC YES!",
    "AMD YES!",
    "tell me why?"};

std::vector<QString> heads = {
    ":/resources/images/head_1.jpg", ":/resources/images/head_2.jpg",
    ":/resources/images/head_3.jpg", ":/resources/images/head_4.jpg",
    ":/resources/images/head_5.jpg",
};

std::vector<QString> names = {"pyf", "lys", "gch",    "lqf",
                              "kyx", "lyy", "潘金金", "潘银银"};

void ChatDialog::addChatUserList() {
    // 创建item
    for (int i = 0; i < 25; i++) {
        int randomValue =
            QRandomGenerator::global()->bounded(100);  // 生成0-99随机整数
        int str_i = randomValue % strs.size();
        int head_i = randomValue % heads.size();
        int name_i = randomValue % names.size();

        auto *chat_user_wid = new ChatUserWid();
        chat_user_wid->SetInfo(names[name_i], heads[head_i], strs[str_i]);
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

void ChatDialog::slot_loading_chat_user() {
    if (_b_loading) {
        return;
    }

    _b_loading = true;
    LoadingDlg *loadingDialog = new LoadingDlg(this);
    loadingDialog->setModal(true);
    loadingDialog->show();
    qDebug() << "add new data to list ...";
    addChatUserList();
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
