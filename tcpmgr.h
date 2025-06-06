#ifndef TCPMGR_H
#define TCPMGR_H
#include <QObject>
#include <QTcpSocket>
#include <functional>

#include "global.h"
#include "singleton.h"
#include "userdata.h"

class TcpMgr : public QObject,
               public Singleton<TcpMgr>,
               public std::enable_shared_from_this<TcpMgr> {
    Q_OBJECT
   public:
    ~TcpMgr();

   private:
    friend class Singleton<TcpMgr>;
    TcpMgr();
    void initHandlers();
    void handleMsg(ReqId id, int len, QByteArray data);
    QTcpSocket _socket;
    QString _host;
    uint16_t _port;
    QByteArray _buffer;
    bool _b_recv_pending;
    quint16 _message_id;
    quint16 _message_len;
    QMap<ReqId, std::function<void(ReqId id, int len, QByteArray data)>>
        _handlers;
   public slots:
    void slot_tcp_connect(ServerInfo);
    void slot_send_data(ReqId reqId, QByteArray data);
   signals:
    void sig_con_success(bool bsuccess);  // 通知其他页面登陆成功
    void sig_send_data(ReqId reqId,
                       QByteArray data);  // 通知发送了数据，阻塞式刷新页面
    void sig_switch_chatdlg();
    void sig_login_failed(int);
    void sig_user_search(std::shared_ptr<SearchInfo>);
    void sig_auth_rsp(std::shared_ptr<AuthRsp>);
    void sig_friend_apply(std::shared_ptr<AddFriendApply>);
    void sig_add_auth_friend(std::shared_ptr<AuthInfo>);
};

#endif  // TCPMGR_H
