#include "searchlist.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "adduseritem.h"
#include "customizeedit.h"
#include "findfaildlg.h"
#include "findsuccessdlg.h"
#include "tcpmgr.h"
#include "userdata.h"
#include "usermgr.h"

SearchList::SearchList(QWidget *parent)
    : QListWidget(parent),
      _find_dlg(nullptr),
      _search_edit(nullptr),
      _send_pending(false) {
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
    connect(this, &QListWidget::itemClicked, this,
            &SearchList::slot_item_clicked);
    addTipItem();
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_user_search, this,
            &SearchList::slot_user_search);
}

void SearchList::CloseFindDlg() {
    if (_find_dlg) {
        _find_dlg->hide();
        _find_dlg = nullptr;
    }
}

void SearchList::SetSearchEdit(QWidget *edit) { _search_edit = edit; }

bool SearchList::eventFilter(QObject *watched, QEvent *event) {
    // 检查事件是否是鼠标悬浮进入或离开
    if (watched == this->viewport()) {
        if (event->type() == QEvent::Enter) {
            // 鼠标悬浮，显示滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        } else if (event->type() == QEvent::Leave) {
            // 鼠标离开，隐藏滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

    // 检查事件是否是鼠标滚轮事件
    if (watched == this->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
        int numDegrees = wheelEvent->angleDelta().y() / 8;
        int numSteps = numDegrees / 15;  // 计算滚动步数

        // 设置滚动幅度
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() -
                                            numSteps);
        return true;
    }

    return QListWidget::eventFilter(watched, event);
}

void SearchList::waitPending(bool pending) {
    if (pending) {
        _loadingDialog = new LoadingDlg(this);
        _loadingDialog->setModal(true);
        _loadingDialog->show();
        _send_pending = pending;
    } else {
        _loadingDialog->hide();
        _loadingDialog->deleteLater();
        _send_pending = pending;
    }
}

void SearchList::addTipItem() {
    auto *INVALID_ITEM = new QWidget();
    QListWidgetItem *item_tmp = new QListWidgetItem;
    // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item_tmp->setSizeHint(QSize(250, 10));
    this->addItem(item_tmp);
    INVALID_ITEM->setObjectName("INVALID_ITEM");
    this->setItemWidget(item_tmp, INVALID_ITEM);
    item_tmp->setFlags(item_tmp->flags() & ~Qt::ItemIsSelectable);
    auto *add_user_item = new AddUserItem();
    QListWidgetItem *item = new QListWidgetItem;
    // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(add_user_item->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, add_user_item);
}

void SearchList::slot_item_clicked(QListWidgetItem *item) {
    QWidget *widget = this->itemWidget(item);
    if (!widget) {
        qDebug("slot item clicked widget is nullptr");
        return;
    }
    ListItemBase *customItem = qobject_cast<ListItemBase *>(widget);
    if (!customItem) {
        qDebug("slot item clicked widget is nullptr");
        return;
    }

    auto itemType = customItem->GetItemType();
    if (itemType == ListItemType::INVALID_ITEM) {
        qDebug("slot invalid item clicked ");
        return;
    }

    if (itemType == ListItemType::ADD_USER_TIP_ITEM) {
        if (_send_pending) {
            return;
        }
        if (_search_edit == nullptr) {
            return;
        }
        waitPending(true);
        auto search_edit = dynamic_cast<CustomizeEdit *>(_search_edit);
        auto uid_str = search_edit->text();
        // 发请求给server
        QJsonObject jsonObj;
        jsonObj["uid"] = uid_str;

        QJsonDocument doc(jsonObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
        emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_SEARCH_USER_REQ,
                                                  jsonData);

        // _find_dlg = std::make_shared<FindSuccessDlg>(this);
        // auto si =
        //     std::make_shared<SearchInfo>(0, "pyf", "pyf", "hey man", 0, "");
        // //_find_dlg是QDialog类的，需要进行动态显式转换
        // std::dynamic_pointer_cast<FindSuccessDlg>(_find_dlg)->SetSearchInfo(si);
        // qDebug("slot ADD_USER_TIP_ITEM clicked ");
        // _find_dlg->show();
        return;
    }
    // 清除弹出框
    CloseFindDlg();
}

void SearchList::slot_user_search(std::shared_ptr<SearchInfo> si) {
    waitPending(false);
    if (si == nullptr) {
        _find_dlg = std::make_shared<FindFailDlg>(this);
    } else {
        auto self_uid = UserMgr::GetInstance()->GetUid();
        if (si->_uid == self_uid) {
            return;  // 不允许搜索自己
        }
        // 分两种情况，一种是收到已经是好友，一种是未添加好友
        bool bExist = UserMgr::GetInstance()->CheckFriendById(si->_uid);
        if (bExist) {
            emit sig_jump_chat_item(si);
            return;
        }
        _find_dlg = std::make_shared<FindSuccessDlg>(this);
        std::dynamic_pointer_cast<FindSuccessDlg>(_find_dlg)->SetSearchInfo(si);
    }

    _find_dlg->show();
}
