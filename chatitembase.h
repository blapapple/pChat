#ifndef CHATITEMBASE_H
#define CHATITEMBASE_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include "global.h"

class BubbleFrame;

class ChatItembase: public QWidget
{
    Q_OBJECT
public:
    explicit ChatItembase(ChatRole role, QWidget *parent = nullptr);
    void setUserName(const QString &name);
    void setUserIcon(const QPixmap &icon);
    void setWidget(QWidget *w);

private:
    ChatRole m_role;
    QLabel *m_pNameLabel;
    QLabel *m_pIconLabel;
    QWidget *m_pBubble;
};

#endif // CHATITEMBASE_H
