#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QPainter>
#include <QPainterPath>
#include "global.h"
/*************************************************************************
 * @file                logindialog.h
 * @brief               登陆界面模块设计
 *
 * @author              pyf
 * @date                2025/05/08
 * @history
 *************************************************************************/
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private:
    QMap<TipErr, QString> _tip_errs;
    Ui::LoginDialog *ui;
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    void InitHead();
    bool CheckUserValid();
    bool CheckPwdValid();
    void AddTipErr(TipErr te, QString tips);
    void DelTipErr(TipErr te);
    void ShowTip(QString str, bool b_ok);
    bool EnableBtn(bool enabled);
    void InitHttpHandlers();
    int _uid;
    QString _token;
public slots:
    void slot_forget_pwd();
signals:
    void SwitchRegister();
    void SwitchReset();
    void sig_connect_tcp(ServerInfo);
private slots:
    void on_btnLogin_clicked();
    void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);
};

#endif // LOGINDIALOG_H
