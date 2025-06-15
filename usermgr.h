#ifndef USERMGR_H
#define USERMGR_H
#include <QMap>
#include <QObject>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

#include "singleton.h"
#include "userdata.h"

class UserMgr : public QObject,
                public Singleton<UserMgr>,
                public std::enable_shared_from_this<UserMgr> {
    Q_OBJECT
   public:
    friend class Singleton<UserMgr>;
    ~UserMgr();
    QString GetName();
    void SetToken(QString token);
    void SetUserInfo(std::shared_ptr<UserInfo> user_info);
    int GetUid();
    std::list<std::shared_ptr<ApplyInfo>> GetApplyList();
    std::vector<std::shared_ptr<FriendInfo>> GetFriendList();
    bool AlreadyApply(int uid);
    bool contains(int uid) const;
    void addOrUpdateApply(std::shared_ptr<ApplyInfo> apply_info);
    void removeApply(int uid);
    void AppendApplyList(QJsonArray array);
    void AppendFriendList(QJsonArray array);
    bool CheckFriendById(int uid);
    void AddFriend(std::shared_ptr<AuthRsp> auth_rsp);
    void AddFriend(std::shared_ptr<AuthInfo> auth_info);
    std::shared_ptr<FriendInfo> GetFriendById(int uid);
    std::list<std::shared_ptr<FriendInfo>> GetChatListPerPage();
    bool IsLoadChatFin();  // 判断是否加载完全
    void UpdateChatLoadedCount();
    std::vector<std::shared_ptr<FriendInfo>> GetConListPerPage();
    void UpdateContactLoadedCount();
    bool IsLoadConFin();

   private:
    UserMgr();
    QString _token;
    std::shared_ptr<UserInfo> _user_info;
    std::list<std::shared_ptr<ApplyInfo>> _apply_list;
    std::unordered_map<int, std::list<std::shared_ptr<ApplyInfo>>::iterator>
        _apply_map;
    QMap<int, std::shared_ptr<FriendInfo>> _friend_map;
    std::vector<std::shared_ptr<FriendInfo>> _friend_list;
    int _chat_loaded;
    int _contact_loaded;
};

#endif  // USERMGR_H
