#ifndef PICTUREBUBBLE_H
#define PICTUREBUBBLE_H
#include <QObject>

#include "bubbleframe.h"
#include "global.h"

class PictureBubble: public BubbleFrame
{
public:
    PictureBubble(const QPixmap &picture, ChatRole role, QWidget *parent = nullptr);
};

#endif // PICTUREBUBBLE_H
