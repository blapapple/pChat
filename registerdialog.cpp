#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"
#include "httpmgr.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    ui->editPasswdReg->setEchoMode(QLineEdit::Password);
    ui->editPasswdConfirmReg->setEchoMode(QLineEdit::Password);
    ui->labelErrorTip->setProperty("state", "normal");
    repolish(ui->labelErrorTip);
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);
    InitHttpHandlers();
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
        qDebug() << "email is " << email;
    });
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
    json_obj["passwd"] = ui->editPasswdReg->text();
    json_obj["confirm"] = ui->editPasswdConfirmReg->text();
    json_obj["varifycode"] = ui->editVerifyCode_reg->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_register"),
                                        json_obj, ReqId::ID_REG_USER, Modules::REGISTERMOD);
}

