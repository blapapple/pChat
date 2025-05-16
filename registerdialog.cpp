#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"
#include "httpmgr.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog), _countdown(5)
{
    ui->setupUi(this);
    ui->editPasswdReg->setEchoMode(QLineEdit::Password);
    ui->editPasswdConfirmReg->setEchoMode(QLineEdit::Password);
    ui->labelErrorTip->setProperty("state", "normal");
    repolish(ui->labelErrorTip);
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);
    InitHttpHandlers();
    ui->labelErrorTip->clear();

    connect(ui->editUserReg, &QLineEdit::editingFinished,this,[this](){
        checkUserValid();
    });

    connect(ui->editEmailReg, &QLineEdit::editingFinished, this, [this](){
        checkEmailValid();
    });

    connect(ui->editPasswdReg, &QLineEdit::editingFinished, this, [this](){
        checkPassValid();
    });

    connect(ui->editPasswdConfirmReg, &QLineEdit::editingFinished, this, [this](){
        checkConfirmValid();
    });

    connect(ui->editVerifyCode_reg, &QLineEdit::editingFinished, this, [this](){
        checkVarifyValid();
    });

    ui->labelPasswdVisible_reg->setCursor(Qt::PointingHandCursor);
    ui->labelPasswdConfirmVisible_reg->setCursor(Qt::PointingHandCursor);

    ui->labelPasswdVisible_reg->SetState("unvisible", "unvisible_hover", "", "visible",
                               "visible_hover", "");

    ui->labelPasswdConfirmVisible_reg->SetState("unvisible", "unvisible_hover", "", "visible",
                                  "visible_hover", "");

    connect(ui->labelPasswdVisible_reg, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->labelPasswdVisible_reg->GetCurrentState();
        if(state == ClickLbState::Normal){
            ui->editPasswdReg->setEchoMode(QLineEdit::Password);
        }else{
            ui->editPasswdReg->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });

    //密码可视切换
    connect(ui->labelPasswdConfirmVisible_reg, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->labelPasswdConfirmVisible_reg->GetCurrentState();
        if(state == ClickLbState::Normal){
            ui->editPasswdConfirmReg->setEchoMode(QLineEdit::Password);
        }else{
            ui->editPasswdConfirmReg->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "Label was clicked!";
    });

    //注册成功后的定时返回逻辑
    _countdown_timer = new QTimer(this);
    connect(_countdown_timer, &QTimer::timeout, [this](){
        if(_countdown == 0){
            _countdown_timer->stop();
            emit sigSwitchLogin();
            return;
        }
        _countdown--;
        auto str = QString("注册成功，%1 s后返回登录").arg(_countdown);
        ui->label_tip->setText(str);
    });

}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_btnVerifyCodeAcq_reg_clicked()
{
    auto email = ui->editEmailReg->text();
    QRegularExpression regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    bool match = regex.match(email).hasMatch();

    if(match){
        //发送http验证码
        QJsonObject json_obj;
        json_obj["email"] = email;
        HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/get_varifycode"),
                                            json_obj, ReqId::ID_GET_VARIFY_CODE, Modules::REGISTERMOD);

    }else{
        ShowTip(tr("邮箱地址不正确"), regStatus::err);
    }
}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        ShowTip(tr("网络请求错误"), regStatus::err);
        return;
    }

    //解析JSON
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if(jsonDoc.isNull()){
        ShowTip(tr("json解析失败"), regStatus::err);
        return;
    }

    //JSON解析错误
    if(!jsonDoc.isObject()){
        ShowTip(tr("json解析对象错误"), regStatus::err);
        return;
    }

    _handlers[id](jsonDoc.object());
    return;
}

void RegisterDialog::ShowTip(QString str, regStatus status)
{
    switch (status) {
    case regStatus::err:
        ui->labelErrorTip->setProperty("state", "err");
        break;
    case regStatus::normal:
        ui->labelErrorTip->setProperty("state", "normal");
        break;
    }

    ui->labelErrorTip->setText(str);
    repolish(ui->labelErrorTip);
}

void RegisterDialog::InitHttpHandlers()
{
    //注册获取验证码回包的逻辑
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE, [this](const QJsonObject& jsonObject){
        int error = jsonObject["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            ShowTip(tr("参数错误"), regStatus::err);
            return;
        }

        auto email = jsonObject["email"].toString();
        ShowTip(tr("验证码已发送"), regStatus::normal);
        qDebug() << "email is" << email;
    });

    //验证注册回包逻辑
    _handlers.insert(ReqId::ID_REG_USER, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            ShowTip(tr("参数错误"), regStatus::err);
            return;
        }
        auto email = jsonObj["email"].toString();
        ShowTip(tr("用户注册成功"), regStatus::normal);
        qDebug() << "user uuid is " << jsonObj["uid"].toString();
        qDebug() << "email is " << email;
        ChangeTipPage();
    });
}

void RegisterDialog::AddTipErr(TipErr te, QString tips){
    _tip_errs[te] = tips;
    ShowTip(tips, regStatus::err);
}

void RegisterDialog::DelTipErr(TipErr te){
    _tip_errs.remove(te);
    if(_tip_errs.empty()){
        ui->labelErrorTip->clear();
        return;
    }

    ShowTip(_tip_errs.first(), regStatus::err);
}

void RegisterDialog::ChangeTipPage()
{
        _countdown_timer->stop();
        ui->stackedWidget->setCurrentWidget(ui->page_2);

        // 启动定时器，设置间隔为1000毫秒（1秒）
        _countdown_timer->start(1000);
}

bool RegisterDialog::checkEmailValid()
{
    //验证邮箱的地址正则表达式
    auto email = ui->editEmailReg->text();
    // 邮箱地址的正则表达式
    QRegularExpression regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
    if(!match){
        //提示邮箱不正确
        AddTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
        return false;
    }

    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool RegisterDialog::checkPassValid()
{
    auto pass = ui->editPasswdReg->text();

    if(pass.length() < 6 || pass.length() > 15){
        //提示长度不准确
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }

    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if(!match){
        //提示字符非法
        AddTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
        return false;;
    }

    DelTipErr(TipErr::TIP_PWD_ERR);

    return true;
}

bool RegisterDialog::checkConfirmValid()
{
    auto pass = ui->editPasswdReg->text();
    auto confirm = ui->editPasswdConfirmReg->text();
    if(pass != confirm){
        AddTipErr(TipErr::TIP_PWD_CONFIRM, tr("确认密码与密码不匹配"));
        return false;
    }else{
        DelTipErr(TipErr::TIP_PWD_CONFIRM);
    }
    return true;
}

bool RegisterDialog::checkVarifyValid()
{
    auto pass = ui->editVerifyCode_reg->text();
    if(pass.isEmpty()){
        AddTipErr(TipErr::TIP_VARIFY_ERR, tr("验证码不能为空"));
        return false;
    }

    DelTipErr(TipErr::TIP_VARIFY_ERR);
    return true;
}


bool RegisterDialog::checkUserValid()
{
    if(ui->editUserReg->text() == ""){
        AddTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
        return false;
    }

    DelTipErr(TipErr::TIP_USER_ERR);
    return true;
}

void RegisterDialog::on_btnSure_reg_clicked()
{

    if(ui->editUserReg->text() == ""){
        ShowTip(tr("用户名不能为空"), regStatus::err);
        return;
    }

    if(ui->editEmailReg->text() == ""){
        ShowTip(tr("邮箱不能为空"), regStatus::err);
        return;
    }

    if(ui->editPasswdReg->text() == ""){
        ShowTip(tr("密码不能为空"), regStatus::err);
        return;
    }

    if(ui->editPasswdConfirmReg->text() == ""){
        ShowTip(tr("确认密码不能为空"), regStatus::err);
        return;
    }

    if(ui->editPasswdConfirmReg->text() != ui->editPasswdReg->text()){
        ShowTip(tr("密码和确认密码不匹配"), regStatus::err);
        return;
    }

    if(ui->editVerifyCode_reg->text() == ""){
        ShowTip(tr("验证码不能为空"), regStatus::err);
        return;
    }

    QJsonObject json_obj;
    json_obj["user"] = ui->editUserReg->text();
    json_obj["email"] = ui->editEmailReg->text();
    json_obj["passwd"] = xorString(ui->editPasswdReg->text());
    json_obj["confirm"] = xorString(ui->editPasswdConfirmReg->text());
    json_obj["varifycode"] = ui->editVerifyCode_reg->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_register"),
                                        json_obj, ReqId::ID_REG_USER, Modules::REGISTERMOD);
}


void RegisterDialog::on_pushButton_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}


void RegisterDialog::on_btnCancel_reg_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}

