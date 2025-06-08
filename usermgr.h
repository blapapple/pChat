#ifndef USERMGR_H
#define USERMGR_H
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
    void SetName(QString name);
    void SetUid(int uid);
    QString GetName();
    void SetToken(QString token);
    void SetUserInfo(std::shared_ptr<UserInfo> user_info);
    int GetUid();
    std::list<std::shared_ptr<ApplyInfo>> GetApplyList();
    bool AlreadyApply(int uid);
    bool contains(int uid) const;
    void addOrUpdateApply(std::shared_ptr<ApplyInfo> apply_info);
    void removeApply(int uid);
    void AppendApplyList(QJsonArray array);

   private:
    UserMgr();
    QString _name;
    QString _token;
    std::shared_ptr<UserInfo> _user_info;
    int _uid;
    std::list<std::shared_ptr<ApplyInfo>> _apply_list;
    std::unordered_map<int, std::list<std::shared_ptr<ApplyInfo>>::iterator>
        _apply_map;
};

#endif  // USERMGR_H
