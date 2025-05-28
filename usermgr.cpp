#include "usermgr.h"

UserMgr::~UserMgr() {}

void UserMgr::SetName(QString name) { _name = name; }

void UserMgr::SetUid(int uid) { _uid = uid; }

QString UserMgr::GetName() { return _name; }

void UserMgr::SetToken(QString token) { _token = token; }

int UserMgr::GetUid() { return _uid; }

std::vector<std::shared_ptr<ApplyInfo> > UserMgr::GetApplyList() {
    return _apply_list;
}

UserMgr::UserMgr() {}
