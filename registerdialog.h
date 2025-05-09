#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "global.h"
/*************************************************************************
 * @file                registerdialog.h
 * @brief               注册页面模块设计
 *
 * @author              pyf
 * @date                2025/05/08
 * @history
 *************************************************************************/

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

    enum class regStatus{
        normal,
        err,
    };

private slots:
    void on_btnVerifyCodeAcq_reg_clicked();
    void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);

private:
    Ui::RegisterDialog *ui;
    void ShowTip(QString str, regStatus status);
    void InitHttpHandlers();
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
};

#endif // REGISTERDIALOG_H
