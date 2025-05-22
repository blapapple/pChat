#ifndef TEXTBUBBLE_H
#define TEXTBUBBLE_H

#include <QObject>
#include <QTextEdit>

#include "bubbleframe.h"

class TextBubble : public BubbleFrame {
    Q_OBJECT
   public:
    TextBubble(ChatRole role, const QString &text, QWidget *parent = nullptr);

   protected:
    bool eventFilter(QObject *o, QEvent *e);  // 过滤气泡绘制时触发的事件
   private:
    void adjustTextHeight();  // 根据行的长度调整气泡高度
    void setPlainText(const QString &text);  // 设置气泡内字符串
    void initStyleSheet();
    QString wraptextToFitWidth(const QString &text, int maxWidth);

    QTextEdit *m_pTextEdit;
    // QString m_text;
};

#endif  // TEXTBUBBLE_H
