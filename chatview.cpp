#include "chatview.h"

#include <QEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStyleOption>

ChatView::ChatView(QWidget *parent) : QWidget(parent), isAppended(false) {
    QVBoxLayout *pMainLayout = new QVBoxLayout();
    this->setLayout(pMainLayout);
    pMainLayout->setContentsMargins(0, 0, 0, 0);

    m_pScrollArea = new QScrollArea();
    m_pScrollArea->setObjectName("chat_area");
    pMainLayout->addWidget(m_pScrollArea);

    QWidget *w = new QWidget(this);
    w->setObjectName("chat_bg");
    w->setAutoFillBackground(true);

    QVBoxLayout *pHLayout_1 = new QVBoxLayout();
    pHLayout_1->addWidget(new QWidget(), 100000);
    w->setLayout(pHLayout_1);
    m_pScrollArea->setWidget(w);

    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScrollBar *pVScrollBar = m_pScrollArea->verticalScrollBar();
    connect(pVScrollBar, &QScrollBar::rangeChanged, this,
            &ChatView::onVScrollBarMoved);

    // 让滚动条浮动在pHLayout_1上方
    QHBoxLayout *pHLayout_2 = new QHBoxLayout();
    pHLayout_2->addWidget(pVScrollBar, 0, Qt::AlignRight);
    pHLayout_2->setContentsMargins(0, 0, 0, 0);
    m_pScrollArea->setLayout(pHLayout_2);
    pVScrollBar->setHidden(true);

    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->installEventFilter(this);
    initStyleSheet();
}

void ChatView::appendChatItem(QWidget *item) {
    QVBoxLayout *vl =
        qobject_cast<QVBoxLayout *>(m_pScrollArea->widget()->layout());
    vl->insertWidget(vl->count() - 1, item);
    isAppended = true;
}

void ChatView::prependChatItem(QWidget *item) {}

void ChatView::insertChatItem(QWidget *before, QWidget *item) {}

void ChatView::removeAllItem() {
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(
        m_pScrollArea->widget()->layout());  // 获得滚动区域的主部件

    // 聊天页面中，最后一个widget是占位widget，其他是聊天气泡widget，
    // 切换聊天页面时需要先将所有聊天气泡widget删除
    int count = layout->count();
    for (int i = 0; i < count - 1; ++i) {
        QLayoutItem *item = layout->takeAt(0);  // 始终以第一个控件开始删除
        if (item) {
            if (QWidget *widget = item->widget()) {
                delete widget;
            }
            delete item;
        }
    }
}

bool ChatView::eventFilter(QObject *o, QEvent *e) {
    if (e->type() == QEvent::Enter && o == m_pScrollArea) {
        // 滚动条大小为0意味着内容没超过滚动区域，不需要滚动条
        m_pScrollArea->verticalScrollBar()->setHidden(
            m_pScrollArea->verticalScrollBar()->maximum() == 0);
    } else if (e->type() == QEvent::Leave && o == m_pScrollArea) {
        m_pScrollArea->verticalScrollBar()->setHidden(true);
    }
    return QWidget::eventFilter(o, e);
}

void ChatView::paintEvent(QPaintEvent *event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatView::onVScrollBarMoved(int min, int max) {
    if (isAppended) {
        QScrollBar *pVScrollBar = m_pScrollArea->verticalScrollBar();
        pVScrollBar->setSliderPosition(pVScrollBar->maximum());
        // 500毫秒可能调用多次
        QTimer::singleShot(500, [this]() { isAppended = false; });
    }
}

void ChatView::initStyleSheet() {}
