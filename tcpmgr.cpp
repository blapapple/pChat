#include "tcpmgr.h"
#include <QAbstractSocket>
#include <QJsonDocument>
#include <QJsonObject>

TcpMgr::~TcpMgr()
{

}

TcpMgr::TcpMgr(): _host(""), _port(0), _b_recv_pending(false), _message_id(0), _message_len(0)
{
    //处理socket连接成功
    QObject::connect(&_socket, &QTcpSocket::connected, [&](){
        qDebug() << "Connected to server!";
        emit sig_con_success(true);
    });

    //处理包到达
    QObject::connect(&_socket, &QTcpSocket::readyRead, [&](){
        //有数据可读，读取所有数据
        //读取所有数据并追加到缓冲区
        _buffer.append(_socket.readAll());
        QDataStream stream(&_buffer, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_6_6);

        forever{
            //第一次接收数据，或者之前接受的数据都是完整的就需要进行下面的判断
            if(!_b_recv_pending){
                //检查缓冲区的数据是否足够解析出一个消息头 (消息ID + 消息长度)
                if(_buffer.size() < static_cast<int>(sizeof(quint16) * 2)){
                    //数据不够，等待buffer取到更多数据
                    return;
                }

                //预读取消息ID和消息长度,但不从缓冲区中移除
                stream >> _message_id >> _message_len;
                //将buffer中前四个字节移除
                _buffer = _buffer.mid(sizeof(quint16) * 2);
                qDebug() << "Message ID: " << _message_id << ", Length: " << _message_len;
            }

            //若缓冲区数据长度不够就继续等待缓冲区取到新数据
            if(_buffer.size() < _message_len){
                _b_recv_pending = true;
                return;
            }

            _b_recv_pending = false;

            //消息头已经在前面被截取了，这里直接截取消息长度就行
            QByteArray messageBody = _buffer.mid(0, _message_len);
            qDebug() << "receive body msg is : " << messageBody;

            _buffer = _buffer.mid(_message_len);
            handleMsg(ReqId(_message_id), _message_len, messageBody);
        }
    });

    //处理错误
    QObject::connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), [&](QAbstractSocket::SocketError socketError){
        Q_UNUSED(socketError)
        qDebug() << "Error: " << _socket.errorString();
        switch (socketError){
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

    //处理连接断开
    QObject::connect(&_socket, &QTcpSocket::disconnected, [&](){
        qDebug() << "Disconnected from server.";
    });
    //连接发送信号来发送数据
    QObject::connect(this, &TcpMgr::sig_send_data, this, &TcpMgr::slot_send_data);
    initHandlers();
}

void TcpMgr::initHandlers()
{
    _handlers.insert(ID_CHAT_LOGIN_RSP, [this](ReqId id, int len, QByteArray data){
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << "data is" << data;

        //转换成json
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        if(jsonDoc.isNull()){
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if(!jsonObj.contains("error")){
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Login failed, err is Json parse err: " << err;
            emit sig_login_failed(err);
            return;
        }

        int err = jsonObj["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Login failed, err is: " << err;
            emit sig_login_failed(err);
            return;
        }

        emit sig_switch_chatdlg();

    });
}

void TcpMgr::handleMsg(ReqId id, int len, QByteArray data)
{
    auto find_iter = _handlers.find(id);
    if(find_iter == _handlers.end()){
        qDebug() << "not found id [" << id << "] to handle";
        return;
    }
    find_iter.value()(id, len, data);
}

void TcpMgr::slot_tcp_connect(ServerInfo si)
{
    qDebug() << "receive tcp connect signal";
    qDebug() << "Connecting to server ...";
    _host = si.Host;
    _port = static_cast<uint16_t>(si.Port.toUInt());
    _socket.connectToHost(_host, _port);
}

void TcpMgr::slot_send_data(ReqId reqId, QString data)
{
    uint16_t id = reqId;

    QByteArray dataBytes = data.toUtf8();
    quint16 len = static_cast<quint16>(dataBytes.size());

    //用于存储要发送的所有数据
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    //设置数据流使用网络字节序
    out.setByteOrder(QDataStream::BigEndian);

    //写入ID和长度
    out << id << len;

    block.append(dataBytes);
    _socket.write(block);
}














