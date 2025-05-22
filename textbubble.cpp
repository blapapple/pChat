#include "textbubble.h"

#include <QEvent>
#include <QFont>
#include <QRectF>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>
#include <QTimer>

TextBubble::TextBubble(ChatRole role, const QString &text, QWidget *parent)
    : BubbleFrame(role, parent) {
    // m_text = text;
    m_pTextEdit = new QTextEdit();
    m_pTextEdit->setReadOnly(true);
    m_pTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pTextEdit->installEventFilter(this);
    // m_pTextEdit->setWordWrapMode(QTextOption::NoWrap);
    QFont font("Microsoft YaHei");
    font.setPointSize(11);
    m_pTextEdit->setFont(font);
    setPlainText(text);
    setWidget(m_pTextEdit);
    initStyleSheet();
    // 延迟设置文本，确保parent布局完成
    // QTimer::singleShot(0, this, [this]() {
    //     if (this->parentWidget()) {
    //         int maxWidth = this->parentWidget()->width() * 0.6;
    //         setPlainText(m_text, maxWidth);
    //     } else {
    //         setPlainText(m_text, 300);  // 若无父组件则设置默认宽度
    //     }
    // });
}

bool TextBubble::eventFilter(QObject *o, QEvent *e) {
    // 如果是绘制气泡时触发的
    if (m_pTextEdit == o && e->type() == QEvent::Paint) {
        // 这里是调整文本高度，由于我们关闭了对应的自动换行策略
        // 需要在这里再次去调用对应的文本填充函数
        adjustTextHeight();

        // 在绘制事件中去调整文本的最大宽度
        // int maxWidth =
        //     this->parentWidget()->width() * 0.6;  // 默认最大宽度为60%
        // setPlainText(m_text, maxWidth);
    }
    return BubbleFrame::eventFilter(o, e);
}

void TextBubble::adjustTextHeight() {
    qreal doc_margin =
        m_pTextEdit->document()->documentMargin();  // 字体到边框的距离默认为4
    QTextDocument *doc = m_pTextEdit->document();
    qreal text_height = 0;
    for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next()) {
        QTextLayout *pLayout = it.layout();
        QRectF text_rect = pLayout->boundingRect();  // 这段的rect
        text_height += text_rect.height();
    }
    int vMargin = this->layout()->contentsMargins().top();
    // 文本高度 + 文本边距 + textedit边框到气泡边框的距离
    setFixedHeight(text_height + doc_margin * 2 + vMargin * 2);
}

void TextBubble::setPlainText(const QString &text) {
    // QString wrappedText = wraptextToFitWidth(text, maxWidth);
    m_pTextEdit->setPlainText(text);

    qreal doc_margin = m_pTextEdit->document()->documentMargin();
    int margin_left = this->layout()->contentsMargins().left();
    int margin_right = this->layout()->contentsMargins().right();
    QFontMetricsF fm(m_pTextEdit->font());
    QTextDocument *doc = m_pTextEdit->document();
    int max_width = 0;
    // 遍历每一行edit，找到最宽的那段
    for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next()) {
        int txtW = int(fm.horizontalAdvance(it.text()));
        max_width = max_width < txtW ? txtW : max_width;
    }
    // 设置最大宽度，只需要设置一次
    setMaximumWidth(max_width + doc_margin * 2 + (margin_left + margin_right) +
                    2);  //+2是因为估计宽度不够导致字符少时会自动换行

    // 设置宽度为传入的maxWidth， 确保文本框自适应
    // setMaximumWidth(maxWidth + doc_margin * 2 + (margin_left +
    // margin_right));
}

void TextBubble::initStyleSheet() {
    m_pTextEdit->setStyleSheet("QTextEdit{background:transparent;border:none}");
}

QString TextBubble::wraptextToFitWidth(const QString &text, int maxWidth) {
    QFontMetricsF metrics(m_pTextEdit->font());
    QString result;
    QStringList lines = text.split('\n');
    for (const QString &line : lines) {
        QString currentLine;
        for (int i = 0; i < line.length(); i++) {
            currentLine.append(line[i]);
            if (metrics.horizontalAdvance(currentLine) > maxWidth) {
                currentLine.chop(1);
                result.append(currentLine + '\n');
                currentLine = line[i];
            }
        }
        result.append(currentLine + '\n');
    }
    return result.trimmed();
}
