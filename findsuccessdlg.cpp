#include "findsuccessdlg.h"

#include <QDir>

#include "ui_findsuccessdlg.h"

FindSuccessDlg::FindSuccessDlg(QWidget *parent)
    : QDialog(parent), ui(new Ui::FindSuccessDlg) {
    ui->setupUi(this);
    // 隐藏对话框标题栏
    setWindowTitle("添加");
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    // 获取当前应用程序的路径
    QString app_path = QCoreApplication::applicationDirPath();
    QString pix_path =
        QDir::toNativeSeparators(app_path + QDir::separator() + "static" +
                                 QDir::separator() + "head_1.jpg");
    QPixmap head_pix(pix_path);
    head_pix = head_pix.scaled(ui->head_label->size(), Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);
    ui->head_label->setPixmap(head_pix);
    ui->add_friend_btn->SetState("normal", "hover", "press");
    this->setModal(true);
}

FindSuccessDlg::~FindSuccessDlg() { delete ui; }

void FindSuccessDlg::SetSearchInfo(std::shared_ptr<SearchInfo> si) {
    ui->name_label->setText(si->_name);
    _si = si;
}

void FindSuccessDlg::on_add_friend_btn_clicked() {
    // todo 添加好友界面弹出
}
