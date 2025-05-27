#include "global.h"

QString gate_url_prefix = "";

std::function<void(QWidget*)> repolish = [](QWidget* w) {
    w->style()->unpolish(w);
    w->style()->polish(w);
};

std::function<QString(QString)> xorString = [](QString input) {
    QString result = input;  // 复制原始字符串便于修改
    int length = input.length();
    length = length % 255;
    for (int i = 0; i < length; ++i) {
        // 对每个字符进行异或操作，假设每个字符都是ASCII，所以直接转换为Qchar
        result[i] = QChar(static_cast<ushort>(input[i].unicode()) ^
                          static_cast<ushort>(length));
    }
    return result;
};

std::vector<QString> strs = {
    "hello world !",
    "nihao",
    "你好你好你好你好你好你好你好你好你好你好你好你好你好你好你好",
    "manba out",
    "what can i say",
    "KFC YES!",
    "AMD YES!",
    "tell me why?"};

std::vector<QString> heads = {
    ":/resources/images/head_1.jpg", ":/resources/images/head_2.jpg",
    ":/resources/images/head_3.jpg", ":/resources/images/head_4.jpg",
    ":/resources/images/head_5.jpg",
};

std::vector<QString> names = {"pyf", "lys", "gch",    "lqf",
                              "kyx", "lyy", "潘金金", "潘银银"};
