#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
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
    Ui::LoginDialog *ui;
signals:
    void SwitchRegister();
};

#endif // LOGINDIALOG_H
