#include "logindialog.h"

#include "httpmgr.h"
#include "tcpmgr.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::LoginDialog) {
    ui->setupUi(this);
    connect(ui->btnReg, &QPushButton::clicked, this,
            &LoginDialog::SwitchRegister);
    ui->labelForget->SetState("normal", "hover", "", "selected",
                              "selected_hover", "");
    connect(ui->labelForget, &ClickedLabel::clicked, this,
            &LoginDialog::slot_forget_pwd);
    InitHead();
    InitHttpHandlers();
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_login_mod_finish, this,
            &LoginDialog::slot_login_mod_finish);

    // tcp连接请求的信号和槽函数
    connect(this, &LoginDialog::sig_connect_tcp, TcpMgr::GetInstance().get(),
            &TcpMgr::slot_tcp_connect);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_con_success, this,
            &LoginDialog::slot_tcp_con_finished);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_login_failed, this,
            &LoginDialog::slot_login_failed);
}

LoginDialog::~LoginDialog() { delete ui; }

void LoginDialog::slot_forget_pwd() {
    qDebug() << "slot forget pwd";
    emit SwitchReset();
}

void LoginDialog::slot_tcp_con_finished(bool bsuccess) {
    if (bsuccess) {
        ShowTip(tr("聊天服务连接成功，正在登录..."), true);
        QJsonObject jsonObj;
        jsonObj["uid"] = _uid;
        jsonObj["token"] = _token;

        QJsonDocument doc(jsonObj);
        QByteArray jsonString = doc.toJson(QJsonDocument::Indented);

        // 发送tcp请求给chat server
        emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_CHAT_LOGIN,
                                                  jsonString);
    } else {
        ShowTip(tr("网络异常"), false);
        EnableBtn(true);
    }
}

void LoginDialog::slot_login_failed(int err) {
    QString result = QString("登陆失败， err is %1").arg(err);
    ShowTip(result, false);
    EnableBtn(true);
}

void LoginDialog::InitHead() {
    QPixmap originalPixmap(":/resources/images/head_1.jpg");
    // 设置图片自动缩放
    qDebug() << originalPixmap.size() << ui->labelLoginHead->size();
    originalPixmap =
        originalPixmap.scaled(ui->labelLoginHead->size(), Qt::KeepAspectRatio,
                              Qt::SmoothTransformation);

    // 创建一个和原始图片相同大小的Qpixmap，用于绘制圆角图片
    QPixmap roundedPixmap(originalPixmap.size());
    roundedPixmap.fill(Qt::transparent);  // 透明

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing);  // 抗锯齿
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.addRoundedRect(0, 0, originalPixmap.width(), originalPixmap.height(),
                        10, 10);  // 设置圆角弧度
    painter.setClipPath(path);

    painter.drawPixmap(0, 0, originalPixmap);      // 绘制原始图片
    ui->labelLoginHead->setPixmap(roundedPixmap);  // 设置圆角图片到Qlabel上
}

bool LoginDialog::EnableBtn(bool enabled) {
    ui->btnLogin->setEnabled(enabled);
    ui->btnReg->setEnabled(enabled);
    return true;
}

void LoginDialog::InitHttpHandlers() {
    _handlers.insert(ReqId::ID_LOGIN_USER, [this](QJsonObject jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS) {
            ShowTip(tr("参数错误"), false);
            EnableBtn(true);
            return;
        }
        auto email = jsonObj["email"].toString();
        ServerInfo si;
        si.Uid = jsonObj["uid"].toInt();
        si.Host = jsonObj["host"].toString();
        si.Token = jsonObj["token"].toString();
        si.Port = jsonObj["port"].toString();

        _uid = si.Uid;
        _token = si.Token;

        qDebug() << "email is " << email << "; uid is " << si.Uid
                 << "; host is " << si.Host << "; port is " << si.Port
                 << "; token is " << si.Token;
        emit sig_connect_tcp(si);
    });
}

bool LoginDialog::CheckUserValid() {
    // 验证邮箱的地址正则表达式
    auto email = ui->editUser->text();
    // 邮箱地址的正则表达式
    QRegularExpression regex(
        R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    bool match = regex.match(email).hasMatch();  // 执行正则表达式匹配
    if (!match) {
        // 提示邮箱不正确
        AddTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
        return false;
    }

    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool LoginDialog::CheckPwdValid() {
    auto pass = ui->editPass->text();

    if (pass.length() < 6 || pass.length() > 15) {
        // 提示长度不准确
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }

    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$
    // 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if (!match) {
        // 提示字符非法
        AddTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
        return false;
        ;
    }

    DelTipErr(TipErr::TIP_PWD_ERR);

    return true;
}

void LoginDialog::on_btnLogin_clicked() {
    qDebug("login btn clicked");
    if (CheckUserValid() == false) {
        return;
    }
    if (CheckPwdValid() == false) {
        return;
    }

    EnableBtn(false);
    auto email = ui->editUser->text();
    auto pwd = ui->editPass->text();

    // 发送http请求登录
    QJsonObject json_obj;
    json_obj["email"] = email;
    json_obj["passwd"] = xorString(pwd);
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/user_login"),
                                        json_obj, ReqId::ID_LOGIN_USER,
                                        Modules::LOGINMOD);
}

void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err) {
    if (err != ErrorCodes::SUCCESS) {
        ShowTip(tr("网络请求错误"), false);
        return;
    }

    // 解析Json字符串，res转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if (jsonDoc.isNull()) {
        ShowTip(tr("json解析错误"), false);
        return;
    }
    if (!jsonDoc.isObject()) {
        ShowTip(tr("json解析错误"), false);
        return;
    }

    _handlers[id](jsonDoc.object());
    return;
}

void LoginDialog::AddTipErr(TipErr te, QString tips) {
    _tip_errs[te] = tips;
    ShowTip(tips, false);
}

void LoginDialog::DelTipErr(TipErr te) {
    _tip_errs.remove(te);
    if (_tip_errs.empty()) {
        ui->err_tip->clear();
        return;
    }

    ShowTip(_tip_errs.first(), false);
}

void LoginDialog::ShowTip(QString str, bool b_ok) {
    if (b_ok) {
        ui->err_tip->setProperty("state", "normal");
    } else {
        ui->err_tip->setProperty("state", "err");
    }

    ui->err_tip->setText(str);

    repolish(ui->err_tip);
}
