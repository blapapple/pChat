#include "tcpmgr.h"

#include <QAbstractSocket>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "usermgr.h"

TcpMgr::~TcpMgr() {}

TcpMgr::TcpMgr()
    : _host(""),
      _port(0),
      _b_recv_pending(false),
      _message_id(0),
      _message_len(0) {
    // 处理socket连接成功
    QObject::connect(&_socket, &QTcpSocket::connected, [&]() {
        qDebug() << "Connected to server!";
        emit sig_con_success(true);
    });

    // 处理包到达
    QObject::connect(&_socket, &QTcpSocket::readyRead, [&]() {
        // 有数据可读，读取所有数据
        // 读取所有数据并追加到缓冲区
        _buffer.append(_socket.readAll());
        QDataStream stream(&_buffer, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_6_6);

        forever {
            // 第一次接收数据，或者之前接受的数据都是完整的就需要进行下面的判断
            if (!_b_recv_pending) {
                // 检查缓冲区的数据是否足够解析出一个消息头 (消息ID + 消息长度)
                if (_buffer.size() < static_cast<int>(sizeof(quint16) * 2)) {
                    // 数据不够，等待buffer取到更多数据
                    return;
                }

                // 预读取消息ID和消息长度,但不从缓冲区中移除
                stream >> _message_id >> _message_len;
                // 将buffer中前四个字节移除
                _buffer = _buffer.mid(sizeof(quint16) * 2);
                qDebug() << "Message ID: " << _message_id
                         << ", Length: " << _message_len;
            }

            // 若缓冲区数据长度不够就继续等待缓冲区取到新数据
            if (_buffer.size() < _message_len) {
                _b_recv_pending = true;
                return;
            }

            _b_recv_pending = false;

            // 消息头已经在前面被截取了，这里直接截取消息长度就行
            QByteArray messageBody = _buffer.mid(0, _message_len);
            qDebug() << "receive body msg is : " << messageBody;

            _buffer = _buffer.mid(_message_len);
            handleMsg(ReqId(_message_id), _message_len, messageBody);
        }
    });

    // 处理错误
    QObject::connect(
        &_socket,
        QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
        [&](QAbstractSocket::SocketError socketError) {
            Q_UNUSED(socketError)
            qDebug() << "Error: " << _socket.errorString();
            switch (socketError) {
                case QTcpSocket::ConnectionRefusedError:
                    qDebug() << "Connection Refused!";
                    emit sig_con_success(false);
                    break;
                case QTcpSocket::RemoteHostClosedError:
                    qDebug() << "Remote Host Closed Connection!";
                    break;
                case QTcpSocket::HostNotFoundError:
                    qDebug() << "Host Not Found!";
                    emit sig_con_success(false);
                    break;
                case QTcpSocket::SocketTimeoutError:
                    qDebug() << "Connection Timeout!";
                    emit sig_con_success(false);
                    break;
                case QTcpSocket::NetworkError:
                    qDebug() << "Network Error!";
                    break;
                default:
                    qDebug() << "Other Error!";
                    break;
            }
        });

    // 处理连接断开
    QObject::connect(&_socket, &QTcpSocket::disconnected,
                     [&]() { qDebug() << "Disconnected from server."; });
    // 连接发送信号来发送数据
    QObject::connect(this, &TcpMgr::sig_send_data, this,
                     &TcpMgr::slot_send_data);
    initHandlers();
}

void TcpMgr::initHandlers() {
    // 每个handler需要处理json，统一处理逻辑
    using JsonHandler = std::function<void(const QJsonObject&, ReqId)>;

    auto MakeJsonHandler = [this](JsonHandler handler,
                                  std::function<void(int)> errSignal) {
        return [this, handler, errSignal](ReqId id, int len, QByteArray data) {
            Q_UNUSED(len);
            qDebug() << "[Handler] ReqId:" << id << "Raw data:" << data;

            // JSON 解析
            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);
            if (jsonDoc.isNull()) {
                qDebug() << "JSON parse error:" << parseError.errorString();
                errSignal(ErrorCodes::ERR_JSON);
                return;
            }

            // 错误字段检查
            QJsonObject jsonObj = jsonDoc.object();
            if (!jsonObj.contains("error")) {
                qDebug() << "Missing 'error' field in response";
                errSignal(ErrorCodes::ERR_JSON);
                return;
            }

            // 业务错误码处理
            int err = jsonObj["error"].toInt();
            if (err != ErrorCodes::SUCCESS) {
                qDebug() << "Business error:" << err;
                errSignal(err);  // 透传业务错误码
                return;
            }

            handler(jsonObj, id);
        };
    };

    // 点击登录后服务器的回复
    _handlers.insert(
        ID_CHAT_LOGIN_RSP,
        MakeJsonHandler(
            [this](const QJsonObject& jsonObj, ReqId id) {
                auto nick = jsonObj["nick"].toString();
                auto icon = jsonObj["icon"].toString();
                auto uid = jsonObj["uid"].toInt();
                auto name = jsonObj["name"].toString();
                auto sex = jsonObj["sex"].toInt();

                auto user_info =
                    std::make_shared<UserInfo>(uid, name, nick, icon, sex);
                UserMgr::GetInstance()->SetUserInfo(user_info);
                UserMgr::GetInstance()->SetToken(jsonObj["token"].toString());

                // 添加申请列表
                if (jsonObj.contains("apply_list")) {
                    UserMgr::GetInstance()->AppendApplyList(
                        jsonObj["apply_list"].toArray());
                }
                // 添加好友列表
                if (jsonObj.contains("friend_list")) {
                    UserMgr::GetInstance()->AppendFriendList(
                        jsonObj["friend_list"].toArray());
                }

                emit sig_switch_chatdlg();
            },
            [this](int err) {
                emit sig_login_failed(err);
                qDebug() << "ID_ADD_FRIEND_RSP Error: " << err;
            }));

    // 点击搜索后服务器的回复
    _handlers.insert(
        ID_SEARCH_USER_RSP,
        MakeJsonHandler(
            [this](const QJsonObject& jsonObj, ReqId id) {
                auto search_info = std::make_shared<SearchInfo>(
                    jsonObj["uid"].toInt(), jsonObj["name"].toString(),
                    jsonObj["nick"].toString(), jsonObj["desc"].toString(),
                    jsonObj["sex"].toInt(), jsonObj["icon"].toString());
                qDebug() << "Search user success, uid is " << search_info->_uid
                         << ", name is " << search_info->_name;

                emit sig_user_search(search_info);
            },
            [this](int err) {
                qDebug() << "ID_SEARCH_USER_RSP Error: " << err;
                emit sig_user_search(nullptr);
            }));

    // 点击申请好友后服务器的回复
    _handlers.insert(ID_ADD_FRIEND_RSP,
                     MakeJsonHandler(
                         [this](const QJsonObject& jsonObj, ReqId id) {
                             qDebug() << "Add Friend RSP Success ";
                         },
                         [this](int err) {
                             qDebug() << "ID_ADD_FRIEND_RSP Error: " << err;
                         }));

    // 被通知有其他客户端传来的好友申请
    _handlers.insert(ID_NOTIFY_ADD_FRIEND_REQ,
                     MakeJsonHandler(
                         [this](const QJsonObject& jsonObj, ReqId id) {
                             int from_uid = jsonObj["applyuid"].toInt();
                             QString name = jsonObj["name"].toString();
                             QString nick = jsonObj["nick"].toString();
                             QString desc = jsonObj["desc"].toString();
                             QString icon = jsonObj["icon"].toString();
                             int sex = jsonObj["sex"].toInt();

                             auto apply_info = std::make_shared<AddFriendApply>(
                                 from_uid, name, desc, icon, nick, sex);
                             emit sig_friend_apply(apply_info);

                             qDebug() << "ID_NOTIFY_ADD_FRIEND_REQ Success ";
                         },
                         [this](int err) {
                             qDebug()
                                 << "ID_NOTIFY_ADD_FRIEND_REQ Error: " << err;
                         }));

    // 被添加方处理好友请求后，服务器给添加方的回复
    _handlers.insert(ID_NOTIFY_AUTH_FRIEND_REQ,
                     MakeJsonHandler(
                         [this](const QJsonObject& jsonObj, ReqId id) {
                             int from_uid = jsonObj["fromuid"].toInt();
                             QString name = jsonObj["name"].toString();
                             QString nick = jsonObj["nick"].toString();
                             QString icon = jsonObj["icon"].toString();
                             int sex = jsonObj["sex"].toInt();

                             auto apply_info = std::make_shared<AuthInfo>(
                                 from_uid, name, nick, icon, sex);
                             emit sig_add_auth_friend(apply_info);

                             qDebug() << "ID_NOTIFY_AUTH_FRIEND_REQ Success ";
                         },
                         [this](int err) {
                             qDebug()
                                 << "ID_NOTIFY_AUTH_FRIEND_REQ Error: " << err;
                         }));

    // 被添加方处理好友认证后的回复
    _handlers.insert(ID_AUTH_FRIEND_RSP,
                     MakeJsonHandler(
                         [this](const QJsonObject& jsonObj, ReqId id) {
                             int uid = jsonObj["uid"].toInt();
                             QString name = jsonObj["name"].toString();
                             QString nick = jsonObj["nick"].toString();
                             QString icon = jsonObj["icon"].toString();
                             int sex = jsonObj["sex"].toInt();

                             auto rsp = std::make_shared<AuthRsp>(
                                 uid, name, nick, icon, sex);
                             emit sig_auth_rsp(rsp);

                             qDebug() << "ID_AUTH_FRIEND_RSP Success ";
                         },
                         [this](int err) {
                             qDebug() << "ID_AUTH_FRIEND_RSP Error: " << err;
                         }));

    // 聊天消息成功到达对方的回包
    _handlers.insert(ID_TEXT_CHAT_MSG_RSP,
                     MakeJsonHandler(
                         [this](const QJsonObject& jsonObj, ReqId id) {
                             qDebug() << "ID_TEXT_CHAT_MSG_RSP Success ";
                         },
                         [this](int err) {
                             qDebug() << "ID_TEXT_CHAT_MSG_RSP Error: " << err;
                         }));

    //  收到聊天消息
    _handlers.insert(
        ID_NOTIFY_TEXT_CHAT_MSG_REQ,
        MakeJsonHandler(
            [this](const QJsonObject& jsonObj, ReqId id) {
                qDebug() << "ID_NOTIFY_TEXT_CHAT_MSG_REQ Success ";
                auto msg_ptr = std::make_shared<TextChatMsg>(
                    jsonObj["fromuid"].toInt(), jsonObj["touid"].toInt(),
                    jsonObj["text_array"].toArray());
                emit sig_text_chat_msg(msg_ptr);
            },
            [this](int err) {
                qDebug() << "ID_NOTIFY_TEXT_CHAT_MSG_REQ Error: " << err;
            }));
}

void TcpMgr::handleMsg(ReqId id, int len, QByteArray data) {
    auto find_iter = _handlers.find(id);
    if (find_iter == _handlers.end()) {
        qDebug() << "not found id [" << id << "] to handle";
        return;
    }
    find_iter.value()(id, len, data);
}

void TcpMgr::slot_tcp_connect(ServerInfo si) {
    qDebug() << "receive tcp connect signal";
    qDebug() << "Connecting to server ...";
    _host = si.Host;
    _port = static_cast<uint16_t>(si.Port.toUInt());
    _socket.connectToHost(_host, _port);
}

void TcpMgr::slot_send_data(ReqId reqId, QByteArray dataBytes) {
    uint16_t id = reqId;

    quint16 len = static_cast<quint16>(dataBytes.size());

    // 用于存储要发送的所有数据
    QByteArray block;
    // out和block绑定，输入到out流中的数据都会到block中
    QDataStream out(&block, QIODevice::WriteOnly);

    // 设置数据流使用网络字节序
    out.setByteOrder(QDataStream::BigEndian);

    // 写入ID和长度
    out << id << len;

    block.append(dataBytes);
    _socket.write(block);
}
