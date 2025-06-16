#include "usermgr.h"

#include <QJsonArray>

UserMgr::~UserMgr() {}

QString UserMgr::GetName() { return _user_info->_name; }

void UserMgr::SetToken(QString token) { _token = token; }

void UserMgr::SetUserInfo(std::shared_ptr<UserInfo> user_info) {
    _user_info = user_info;
}

int UserMgr::GetUid() { return _user_info->_uid; }

std::list<std::shared_ptr<ApplyInfo>> UserMgr::GetApplyList() {
    return _apply_list;
}

std::vector<std::shared_ptr<FriendInfo>> UserMgr::GetFriendList() {
    return _friend_list;
}

bool UserMgr::AlreadyApply(int uid) {
    auto it = _apply_map.find(uid);
    if (it != _apply_map.end()) {
        return true;
    }

    return false;
}

void UserMgr::addOrUpdateApply(std::shared_ptr<ApplyInfo> apply_info) {  //
    auto it = _apply_map.find(apply_info->_uid);
    if (it != _apply_map.end()) {
        _apply_list.erase(it->second);
        _apply_map.erase(it);
    }
    _apply_list.emplace_front(apply_info);
    _apply_map[apply_info->_uid] = _apply_list.begin();
}

void UserMgr::removeApply(int uid) {
    auto it = _apply_map.find(uid);
    if (it != _apply_map.end()) {
        _apply_list.erase(it->second);
        _apply_map.erase(it);
    }
}

void UserMgr::AppendApplyList(QJsonArray array) {
    for (const QJsonValue& value : array) {
        auto name = value["name"].toString();
        auto desc = value["desc"].toString();
        auto icon = value["icon"].toString();
        auto nick = value["nick"].toString();
        auto uid = value["uid"].toInt();
        auto sex = value["sex"].toInt();
        auto status = value["status"].toInt();
        auto info = std::make_shared<ApplyInfo>(uid, name, desc, icon, nick,
                                                sex, status);
        _apply_list.emplace_back(info);
    }
}

void UserMgr::AppendFriendList(QJsonArray array) {
    for (const QJsonValue& value : array) {
        auto name = value["name"].toString();
        auto desc = value["desc"].toString();
        auto icon = value["icon"].toString();
        auto nick = value["nick"].toString();
        auto uid = value["uid"].toInt();
        auto sex = value["sex"].toInt();
        auto back = value["back"].toString();
        auto info = std::make_shared<FriendInfo>(uid, name, nick, icon, sex,
                                                 desc, back);
        _friend_map.insert(uid, info);
        _friend_list.emplace_back(info);
    }
}

bool UserMgr::CheckFriendById(int uid) {
    auto iter = _friend_map.find(uid);
    if (iter == _friend_map.end()) {
        return false;
    }
    return true;
}

void UserMgr::AddFriend(std::shared_ptr<AuthRsp> auth_rsp) {
    auto friend_info = std::make_shared<FriendInfo>(auth_rsp);
    _friend_map[friend_info->_uid] = friend_info;
}

void UserMgr::AddFriend(std::shared_ptr<AuthInfo> auth_info) {
    auto friend_info = std::make_shared<FriendInfo>(auth_info);
    _friend_map[friend_info->_uid] = friend_info;
}

std::shared_ptr<FriendInfo> UserMgr::GetFriendById(int uid) {
    auto iter = _friend_map.find(uid);
    if (iter == _friend_map.end()) {
        return nullptr;
    }
    // 迭代器并不是FriendInfo，需要取迭代器的值才是FriendInfo的指针
    return *iter;
}

std::list<std::shared_ptr<FriendInfo>> UserMgr::GetChatListPerPage() {
    std::list<std::shared_ptr<FriendInfo>> friend_list;
    int begin = _chat_loaded;
    int end = begin + CHAT_COUNT_PER_PAGE;

    if (begin >= _friend_list.size()) {
        return friend_list;
    }

    if (end > _friend_list.size()) {
        friend_list = std::list<std::shared_ptr<FriendInfo>>(
            _friend_list.begin() + begin, _friend_list.end());
        return friend_list;
    }

    friend_list = std::list<std::shared_ptr<FriendInfo>>(
        _friend_list.begin() + begin, _friend_list.begin() + end);
    return friend_list;
}

bool UserMgr::IsLoadChatFin() {
    if (_chat_loaded >= _friend_list.size()) {
        return true;
    }

    return false;
}

void UserMgr::UpdateChatLoadedCount() {
    int begin = _chat_loaded;
    int end = begin + CHAT_COUNT_PER_PAGE;

    if (begin >= _friend_list.size()) {
        return;
    }

    if (end > _friend_list.size()) {
        _chat_loaded = _friend_list.size();
        return;
    }

    _chat_loaded = end;
}

std::vector<std::shared_ptr<FriendInfo>> UserMgr::GetConListPerPage() {
    std::vector<std::shared_ptr<FriendInfo>> friend_list;
    int begin = _contact_loaded;
    int end = begin + CHAT_COUNT_PER_PAGE;

    if (begin >= _friend_list.size()) {
        return friend_list;
    }

    if (end > _friend_list.size()) {
        friend_list = std::vector<std::shared_ptr<FriendInfo>>(
            _friend_list.begin() + begin, _friend_list.end());
        return friend_list;
    }

    friend_list = std::vector<std::shared_ptr<FriendInfo>>(
        _friend_list.begin() + begin, _friend_list.begin() + end);
    return friend_list;
}

void UserMgr::UpdateContactLoadedCount() {
    int begin = _contact_loaded;
    int end = _contact_loaded + CHAT_COUNT_PER_PAGE;

    if (begin >= _friend_list.size()) {
        return;
    }
    if (end > _friend_list.size()) {
        _contact_loaded = _friend_list.size();
        return;
    }
    _contact_loaded = end;
}

bool UserMgr::IsLoadConFin() {
    if (_contact_loaded >= _friend_list.size()) {
        return true;
    }

    return false;
}

std::shared_ptr<UserInfo> UserMgr::GetUserInfo() { return _user_info; }

void UserMgr::AppendFriendChatMsg(
    int friend_id, std::vector<std::shared_ptr<TextChatData>> msgs) {
    auto find_iter = _friend_map.find(friend_id);
    if (find_iter == _friend_map.end()) {
        qDebug() << "append friend uid " << friend_id << " not found ";
        return;
    }

    find_iter.value()->AppendChatMsgs(msgs);
}

bool UserMgr::contains(int uid) const {
    return _apply_map.find(uid) != _apply_map.end();
}

UserMgr::UserMgr() : _user_info(nullptr), _chat_loaded(0), _contact_loaded(0) {}
