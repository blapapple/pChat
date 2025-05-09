#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connect(ui->btnReg, &QPushButton::clicked, this, &LoginDialog::SwitchRegister);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}
