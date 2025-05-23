#ifndef SEARCHLIST_H
#define SEARCHLIST_H

#include <QDebug>
#include <QEvent>
#include <QListWidget>
#include <QObject>
#include <QScrollBar>
#include <QWheelEvent>
#include <memory>

#include "loadingdlg.h"
#include "userdata.h"

class SearchList : public QListWidget {
    Q_OBJECT
   public:
    SearchList(QWidget *parent = nullptr);
    void CloseFindDlg();
    void SetSearchEdit(QWidget *edit);

   protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

   private:
    void waitPending(bool pending = true);
    bool _send_pending;  // 阻塞，好友申请时等待服务器回包
    void addTipItem();
    std::shared_ptr<QDialog> _find_dlg;
    QWidget *_search_edit;
    LoadingDlg *_loadingDialog;
   private slots:
    void slot_item_clicked(QListWidgetItem *item);
    void slot_user_search(std::shared_ptr<SearchInfo> si);
   signals:
};

#endif  // SEARCHLIST_H
