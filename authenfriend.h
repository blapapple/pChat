#ifndef AUTHENFRIEND_H
#define AUTHENFRIEND_H

#include <QDialog>

#include "clickedlabel.h"
#include "friendlabel.h"
#include "userdata.h"

namespace Ui {
class AuthenFriend;
}

class AuthenFriend : public QDialog {
    Q_OBJECT

   public:
    explicit AuthenFriend(QWidget *parent = nullptr);
    ~AuthenFriend();
    void InitTipLbs();
    void AddTipLbs(ClickedLabel *, QPoint cur_point, QPoint &next_point,
                   int text_width, int text_height);
    // 重写事件过滤器，展示滚动条
    bool eventFilter(QObject *obj, QEvent *event);
    // 当服务器传来用户数据时的处理
    void SetSearchInfo(std::shared_ptr<SearchInfo> si);
    void SetApplyInfo(std::shared_ptr<ApplyInfo> apply_info);

   private:
    Ui::AuthenFriend *ui;
    // 重排好友标签编辑栏的标签
    void resetLabels();
    std::shared_ptr<ApplyInfo> _apply_info;

    // 已经创建好的标签
    QMap<QString, ClickedLabel *> _add_labels;
    std::vector<QString> _add_label_keys;
    QPoint _label_point;
    // 用来在输入框显示添加新好友的标签
    QMap<QString, FriendLabel *> _friend_labels;
    std::vector<QString> _friend_label_keys;
    void addLabel(QString name);
    std::vector<QString> _tip_data;
    QPoint _tip_cur_point;
    std::shared_ptr<SearchInfo> _si;
   public slots:
    // 显示更多label标签
    void ShowMoreLabel();
    // 输入label按下回车触发将标签加入展示栏
    void slot_label_enter();
    // 点击关闭，移除展示栏好友便签
    void slot_remove_friend_label(QString);
    // 通过点击tip实现增加和减少好友便签
    void slot_change_friend_label_by_tip(QString, ClickLbState);
    // 输入框文本变化显示不同提示
    void slot_label_text_change(const QString &text);
    // 输入框输入完成
    void slot_label_edit_finished();
    // 输入标签显示提示框，点击提示框内容后添加好友便签
    void slot_add_friend_label_by_click_tip(QString text);
    // 处理确认回调
    void slot_apply_sure();
    // 处理取消回调
    void slot_apply_cancel();
};

#endif  // AUTHENFRIEND_H
