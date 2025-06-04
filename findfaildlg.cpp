#include "findfaildlg.h"

#include <QDebug>

#include "ui_findfaildlg.h"

FindFailDlg::FindFailDlg(QWidget *parent)
    : QDialog(parent), ui(new Ui::FindFailDlg) {
    ui->setupUi(this);
    setWindowTitle("添加");
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("FindFailDlg");
    ui->fail_sure_btn->SetState("normal", "hover", "press");
    this->setModal(true);
}

FindFailDlg::~FindFailDlg() {
    qDebug() << "FindFailDlg destruct";
    delete ui;
}

void FindFailDlg::on_fail_sure_btn_clicked() {}
