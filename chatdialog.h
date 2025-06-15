#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <clickedbtn.h>

#include <QDialog>
#include <QList>
#include <QListWidgetItem>
#include <QMap>
#include <unordered_map>

#include "global.h"
#include "statewidget.h"
#include "userdata.h"

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog {
    Q_OBJECT

   public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    void addChatUserList();
    void ClearLabelState(
        StateWidget
            *lb);  // 好像是清除侧边label的状态，那选中之后是不是得吧红点去掉？

   protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void handleGlobalMousePress(QMouseEvent *);

   private:
    void SetSelectChatItem(int uid);
    void SetSelectChatPage(int uid = 0);
    void ShowSearch(bool bsearch);
    void AddLBGroup(StateWidget *lb);
    void loadMoreChatUser();
    void loadMoreConUser();

    Ui::ChatDialog *ui;
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    QList<StateWidget *> _lb_list;
    QMap<int, QListWidgetItem *> _chat_items_added;
    int _cur_chat_uid;

   private slots:
    void slot_loading_chat_user();
    void slot_side_chat();
    void slot_side_contact();
    void slot_text_changed(const QString &str);
    void slot_loading_contact_user();
   public slots:
    void slot_apply_friend(std::shared_ptr<AddFriendApply> apply);
    void slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info);
    void slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp);
    void slot_jump_chat_item(std::shared_ptr<SearchInfo> si);
};

#endif  // CHATDIALOG_H
