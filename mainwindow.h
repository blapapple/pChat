#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logindialog.h"
#include "registerdialog.h"
#include "resetdialog.h"
#include "chatdialog.h"
/*************************************************************************
 * @file                mainwindow.h
 * @brief               主窗口
 *
 * @author              pyf
 * @date                2025/05/07
 * @history
 *************************************************************************/

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void SlotSwitchReg();
    void SlotSwitchLogin();
    void SlotSwitchReset();
    void SlotSwitchLoginFromReset();
    void SlotSwitchChat();
private:
    Ui::MainWindow *ui;
    LoginDialog *_login_dlg = nullptr;
    RegisterDialog *_reg_dlg = nullptr;
    ResetDialog* _reset_dlg = nullptr;
    ChatDialog* _chat_dlg = nullptr;
};
#endif // MAINWINDOW_H
