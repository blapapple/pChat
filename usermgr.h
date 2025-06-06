#ifndef USERMGR_H
#define USERMGR_H
#include <QObject>
#include <memory>
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
    int GetUid();
    std::vector<std::shared_ptr<ApplyInfo>> GetApplyList();

   private:
    UserMgr();
    QString _name;
    QString _token;
    int _uid;
    std::vector<std::shared_ptr<ApplyInfo>> _apply_list;
};

#endif  // USERMGR_H
