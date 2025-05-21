#include "customizeedit.h"

CustomizeEdit::CustomizeEdit(QWidget *parent):QLineEdit(parent), _max_len(0)
{
    connect(this, &QLineEdit::textChanged, this, &CustomizeEdit::limitTextLength);
}

void CustomizeEdit::SetMaxLength(int maxlen)
{
    _max_len = maxlen;
}

void CustomizeEdit::focusOutEvent(QFocusEvent *event)
{
    //自定义失去焦点时的处理逻辑
    QLineEdit::focusOutEvent(event);
    emit sig_foucus_out();
}

void CustomizeEdit::limitTextLength(QString text)
{
    if(_max_len <= 0){
        return;
    }
    QByteArray byteArray = text.toUtf8();
    if(byteArray.size() > _max_len){
        byteArray = byteArray.left(_max_len); //限制长度
        this->setText(QString::fromUtf8(byteArray));
    }
}
