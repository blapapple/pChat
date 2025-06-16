#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <clickedbtn.h>

#include <QDialog>
#include <QList>
#include <QListWidgetItem>
#include <QMap>
#include <unordered_map>
#include <vector>

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
    void SetSelectChatItem(int uid = 0);
    void SetSelectChatPage(int uid = 0);
    void ShowSearch(bool bsearch);
    void AddLBGroup(StateWidget *lb);
    void loadMoreChatUser();
    void loadMoreConUser();
    void UpdateChatMsg(std::vector<std::shared_ptr<TextChatData>> msgdata);

    Ui::ChatDialog *ui;
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    QList<StateWidget *> _lb_list;
    QMap<int, QListWidgetItem *> _chat_items_added;
    int _cur_chat_uid;
    QWidget *_last_widget;

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
    void slot_friend_info_page(std::shared_ptr<FriendInfo> user_info);
    void slot_switch_apply_friend_page();
    void slot_jump_chat_item_from_infopage(
        std::shared_ptr<FriendInfo> user_info);
    void slot_item_clicked(QListWidgetItem *item);
    void slot_append_send_chat_msg(std::shared_ptr<TextChatData> msgdata);
    void slot_text_chat_msg(std::shared_ptr<TextChatMsg>);
};

#endif  // CHATDIALOG_H
