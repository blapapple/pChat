#include "chatuserlist.h"

#include <QApplication>
#include <QTimer>

#include "usermgr.h"

ChatUserList::ChatUserList(QWidget *parent)
    : QListWidget(parent), _load_pending(false) {
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->viewport()->installEventFilter(this);
}

bool ChatUserList::eventFilter(QObject *watched, QEvent *event) {
    // 检查鼠标是否悬浮进入或离开
    if (watched == this->viewport()) {
        if (event->type() == QEvent::Enter) {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        } else if (event->type() == QEvent::Leave) {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

    // 检查事件是否是鼠标滚轮事件
    if (watched == this->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
        int numDegrees = wheelEvent->angleDelta().y() / 8;
        int numSteps = numDegrees / 15;  // 滚动步数

        // 设置滚动幅度
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() -
                                            numSteps);

        // 检查是否滚动到底部
        QScrollBar *scrollBar = this->verticalScrollBar();
        int maxScrollValue = scrollBar->maximum();
        int currentValue = scrollBar->value();
        // int pageSize = 10; //每页加载的联系人数量
        if (maxScrollValue - currentValue <= 0) {
            auto b_loaded = UserMgr::GetInstance()->IsLoadConFin();
            //  确保上一次加载完成后再进行下面加载
            if (b_loaded) {
                return true;
            }
            if (_load_pending) {
                return true;
            }
            // 滚动到底部，加载新的联系人
            qDebug() << "load more chat user";
            _load_pending = true;

            QTimer::singleShot(100, [this]() {
                _load_pending = false;
                QCoreApplication::quit();
            });
            emit sig_loading_chat_user();
        }
        return true;  // 停止事件传递
    }

    return QListWidget::eventFilter(watched, event);
}
