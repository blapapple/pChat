#ifndef CHATUSERLIST_H
#define CHATUSERLIST_H
#include <QObject>
#include <QWheelEvent>
#include <QListWidget>
#include <QScrollBar>
#include <QEvent>

class ChatUserList: public QListWidget
{
    Q_OBJECT
public:
    ChatUserList(QWidget *parent);
protected:
    //重写来实现自己需要的事件处理操作
    bool eventFilter(QObject *watched, QEvent *event) override;
signals:
    void sig_loading_chat_user();
};

#endif // CHATUSERLIST_H
