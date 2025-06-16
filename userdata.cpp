#include "userdata.h"

#include <QJsonObject>

SearchInfo::SearchInfo(int uid, QString name, QString nick, QString desc,
                       int sex, QString icon)
    : _uid(uid),
      _name(name),
      _nick(nick),
      _desc(desc),
      _sex(sex),
      _icon(icon) {}

AddFriendApply::AddFriendApply(int from_uid, QString name, QString desc,
                               QString icon, QString nick, int sex)
    : _from_uid(from_uid),
      _name(name),
      _desc(desc),
      _icon(icon),
      _nick(nick),
      _sex(sex) {}

TextChatMsg::TextChatMsg(int fromuid, int touid, QJsonArray arrays)
    : _from_uid(fromuid), _to_uid(touid) {
    for (auto msg_data : arrays) {
        auto msg_obj = msg_data.toObject();
        auto content = msg_obj["content"].toString();
        auto msgid = msg_obj["msgid"].toString();
        auto msg_ptr =
            std::make_shared<TextChatData>(msgid, content, fromuid, touid);
        _chat_msgs.emplace_back(msg_ptr);
    }
}

void FriendInfo::AppendChatMsgs(
    const std::vector<std::shared_ptr<TextChatData> > text_vec) {
    for (const auto& text : text_vec) {
        _chat_msgs.emplace_back(text);
    }
}
