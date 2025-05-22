#ifndef BUBBLEFRAME_H
#define BUBBLEFRAME_H

#include <QObject>
#include <QFrame>
#include "global.h"
#include <QHBoxLayout>

//自定义气泡
class BubbleFrame: public QFrame
{
    Q_OBJECT
public:
    BubbleFrame(ChatRole role, QWidget *parent = nullptr);
    void setMargin(int margin);
    void setWidget(QWidget *w);//文本或图片的设置
protected:
    void paintEvent(QPaintEvent *e);
private:
    QHBoxLayout *m_pHLayout;
    ChatRole m_role;
    int m_margin;
};

#endif // BUBBLEFRAME_H
