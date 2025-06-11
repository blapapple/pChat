#include "authenfriend.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QScrollBar>

#include "tcpmgr.h"
#include "ui_authenfriend.h"
#include "usermgr.h"

AuthenFriend::AuthenFriend(QWidget* parent)
    : QDialog(parent), ui(new Ui::AuthenFriend), _label_point(2, 6) {
    ui->setupUi(this);

    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("AuthenFriend");
    this->setModal(true);
    ui->label_ed->setPlaceholderText(tr("搜索、添加标签"));
    ui->back_ed->setPlaceholderText(tr("潘银银"));

    ui->label_ed->setMaxLength(21);
    ui->label_ed->move(2, 2);
    ui->label_ed->setFixedHeight(20);
    ui->label_ed->setMaxLength(10);
    ui->input_tip_widget->hide();

    _tip_cur_point = QPoint(5, 5);
    _tip_data = {"大学", "初中", "高中", "朋友", "家人",   "同事", "c++",
                 "java", "rust", "go",   "圆神", "小马哥", "小白", "小红",
                 "小绿", "小蓝", "小黄", "小黑", "小紫",   "小橙", "小灰",
                 "小粉", "小金", "小银", "小铜"};

    connect(ui->more_lb, &ClickedOnceLabel::clicked, this,
            &AuthenFriend::ShowMoreLabel);
    InitTipLbs();
    // 链接输入标签回车事件
    connect(ui->label_ed, &CustomizeEdit::returnPressed, this,
            &AuthenFriend::slot_label_enter);
    connect(ui->label_ed, &CustomizeEdit::textChanged, this,
            &AuthenFriend::slot_label_text_change);
    connect(ui->label_ed, &CustomizeEdit::editingFinished, this,
            &AuthenFriend::slot_label_edit_finished);
    connect(ui->tip_lb, &ClickedOnceLabel::clicked, this,
            &AuthenFriend::slot_add_friend_label_by_click_tip);
    //  设置输入框的样式
    ui->scrollArea->horizontalScrollBar()->setHidden(true);
    ui->scrollArea->verticalScrollBar()->setHidden(true);
    ui->scrollArea->installEventFilter(this);
    ui->sure_btn->SetState("normal", "hover", "press");
    ui->cancel_btn->SetState("normal", "hover", "press");
    // 连接确认和取消按钮的槽函数
    connect(ui->cancel_btn, &QPushButton::clicked, this,
            &AuthenFriend::slot_apply_cancel);
    connect(ui->sure_btn, &QPushButton::clicked, this,
            &AuthenFriend::slot_apply_sure);
}

AuthenFriend::~AuthenFriend() {
    qDebug() << "AuthenFriend destructed";
    delete ui;
}

void AuthenFriend::InitTipLbs() {
    int lines = 1;
    for (int i = 0; i < _tip_data.size(); i++) {
        auto* lb = new ClickedLabel(ui->lb_list);
        lb->SetState("normal", "hover", "pressed", "selected_normal",
                     "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(_tip_data[i]);
        lb->setContentsMargins({4, 1, 4, 1});
        connect(lb, &ClickedLabel::clicked, this,
                &AuthenFriend::slot_change_friend_label_by_tip);
        QFontMetrics fontMetrics(lb->font());  // 获取QLabel控件的字体信息
        int textWidth =
            fontMetrics.horizontalAdvance(lb->text());  // 获取文本的宽度
        int textHeight = fontMetrics.height();          // 获取文本的高度
        if (_tip_cur_point.x() + textWidth + tip_offset >
            ui->lb_list->width()) {
            lines++;
            if (lines > 2) {
                delete lb;
                return;
            }
            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);
        }
        auto next_point = _tip_cur_point;
        AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);
        _tip_cur_point = next_point;
    }
}

void AuthenFriend::AddTipLbs(ClickedLabel* lb, QPoint cur_point,
                             QPoint& next_point, int text_width,
                             int text_height) {
    lb->move(cur_point);
    lb->show();
    lb->setContentsMargins({4, 1, 4, 1});
    _add_labels.insert(lb->text(), lb);
    _add_label_keys.push_back(lb->text());
    next_point.setX(lb->pos().x() + text_width + 15);
    next_point.setY(lb->pos().y());
}

bool AuthenFriend::eventFilter(QObject* obj, QEvent* event) {
    if (obj == ui->scrollArea && event->type() == QEvent::Enter) {
        ui->scrollArea->verticalScrollBar()->setHidden(false);
    } else if (obj == ui->scrollArea && event->type() == QEvent::Leave) {
        ui->scrollArea->verticalScrollBar()->setHidden(true);
    }
    return QObject::eventFilter(obj, event);
}

void AuthenFriend::SetSearchInfo(std::shared_ptr<SearchInfo> si) {
    _si = si;
    auto applyname = UserMgr::GetInstance()->GetName();
    auto bakname = si->_name;
    ui->back_ed->setText(bakname);
}

void AuthenFriend::SetApplyInfo(std::shared_ptr<ApplyInfo> apply_info) {
    _apply_info = apply_info;
    ui->back_ed->setPlaceholderText(apply_info->_name);
}

void AuthenFriend::ShowMoreLabel() {
    qDebug() << "receive more label clicked";
    ui->more_lb_wid->hide();
    ui->lb_list->setFixedWidth(300);
    _tip_cur_point = QPoint(5, 5);
    auto next_point = _tip_cur_point;
    int textWidth;
    int textHeight;
    // 重拍现有的label
    for (auto& added_key : _add_label_keys) {
        auto added_lb = _add_labels[added_key];
        QFontMetrics fontMetrics(added_lb->font());  // 获取QLabel控件的字体信息
        textWidth =
            fontMetrics.horizontalAdvance(added_lb->text());  // 获取文本的宽度
        textHeight = fontMetrics.height();  // 获取文本的高度
        if (_tip_cur_point.x() + textWidth + tip_offset >
            ui->lb_list->width()) {
            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);
        }
        added_lb->move(_tip_cur_point);
        next_point.setX(added_lb->pos().x() + textWidth + 15);
        next_point.setY(_tip_cur_point.y());
        _tip_cur_point = next_point;
    }
    // 添加未添加的
    for (int i = 0; i < _tip_data.size(); i++) {
        auto iter = _add_labels.find(_tip_data[i]);
        if (iter != _add_labels.end()) {
            continue;
        }
        auto* lb = new ClickedLabel(ui->lb_list);
        lb->SetState("normal", "hover", "pressed", "selected_normal",
                     "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(_tip_data[i]);
        connect(lb, &ClickedLabel::clicked, this,
                &AuthenFriend::slot_change_friend_label_by_tip);
        QFontMetrics fontMetrics(lb->font());  // 获取QLabel控件的字体信息
        int textWidth =
            fontMetrics.horizontalAdvance(lb->text());  // 获取文本的宽度
        int textHeight = fontMetrics.height();          // 获取文本的高度
        if (_tip_cur_point.x() + textWidth + tip_offset >
            ui->lb_list->width()) {
            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);
        }
        next_point = _tip_cur_point;
        AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);
        _tip_cur_point = next_point;
    }
    int diff_height =
        next_point.y() + textHeight + tip_offset - ui->lb_list->height();
    ui->lb_list->setFixedHeight(next_point.y() + textHeight + tip_offset);
    // qDebug()<<"after resize ui->lb_list size is " <<  ui->lb_list->size();
    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height() +
                                      diff_height);
}

void AuthenFriend::resetLabels() {
    auto max_width = ui->grid_widget->width();
    auto label_height = 0;
    for (auto iter = _friend_labels.begin(); iter != _friend_labels.end();
         iter++) {
        // todo... 添加宽度统计
        if (_label_point.x() + iter.value()->width() > max_width) {
            _label_point.setY(_label_point.y() + iter.value()->height() + 6);
            _label_point.setX(2);
        }
        iter.value()->move(_label_point);
        iter.value()->show();
        _label_point.setX(_label_point.x() + iter.value()->width() + 2);
        _label_point.setY(_label_point.y());
        label_height = iter.value()->height();
    }
    if (_friend_labels.isEmpty()) {
        ui->label_ed->move(_label_point);
        return;
    }
    if (_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->grid_widget->width()) {
        ui->label_ed->move(2, _label_point.y() + label_height + 6);
    } else {
        ui->label_ed->move(_label_point);
    }
}

void AuthenFriend::addLabel(QString name) {
    if (_friend_labels.find(name) != _friend_labels.end()) {
        return;
    }
    auto tmplabel = new FriendLabel(ui->grid_widget);
    tmplabel->SetText(name);
    tmplabel->setObjectName("FriendLabel");
    tmplabel->setContentsMargins({4, 1, 4, 1});
    auto max_width = ui->grid_widget->width();
    // todo... 添加宽度统计
    if (_label_point.x() + tmplabel->width() > max_width) {
        _label_point.setY(_label_point.y() + tmplabel->height() + 6);
        _label_point.setX(2);
    } else {
    }
    tmplabel->move(_label_point);
    tmplabel->show();
    _friend_labels[tmplabel->Text()] = tmplabel;
    _friend_label_keys.push_back(tmplabel->Text());
    connect(tmplabel, &FriendLabel::sig_close, this,
            &AuthenFriend::slot_remove_friend_label);
    _label_point.setX(_label_point.x() + tmplabel->width() + 2);
    if (_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->grid_widget->width()) {
        ui->label_ed->move(2, _label_point.y() + tmplabel->height() + 2);
    } else {
        ui->label_ed->move(_label_point);
    }
    ui->label_ed->clear();
    if (ui->grid_widget->height() < _label_point.y() + tmplabel->height() + 2) {
        ui->grid_widget->setFixedHeight(_label_point.y() +
                                        tmplabel->height() * 2 + 2);
    }
}

void AuthenFriend::slot_label_enter() {
    if (ui->label_ed->text().isEmpty()) {
        return;
    }
    auto text = ui->label_ed->text();
    addLabel(ui->label_ed->text());
    ui->input_tip_widget->hide();
    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    // 找到了就只需设置状态为选中即可
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }
    // 判断标签展示栏是否有该标签
    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->SetCurState(ClickLbState::Selected);
        return;
    }
    // 标签展示栏也增加一个标签, 并设置绿色选中
    auto* lb = new ClickedLabel(ui->lb_list);
    lb->SetState("normal", "hover", "pressed", "selected_normal",
                 "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &ClickedLabel::clicked, this,
            &AuthenFriend::slot_change_friend_label_by_tip);
    qDebug() << "ui->lb_list->width() is " << ui->lb_list->width();
    qDebug() << "_tip_cur_point.x() is " << _tip_cur_point.x();
    QFontMetrics fontMetrics(lb->font());  // 获取QLabel控件的字体信息
    int textWidth =
        fontMetrics.horizontalAdvance(lb->text());  // 获取文本的宽度
    int textHeight = fontMetrics.height();          // 获取文本的高度
    qDebug() << "textWidth is " << textWidth;
    if (_tip_cur_point.x() + textWidth + tip_offset + 3 >
        ui->lb_list->width()) {
        _tip_cur_point.setX(5);
        _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);
    }
    auto next_point = _tip_cur_point;
    AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);
    _tip_cur_point = next_point;
    int diff_height =
        next_point.y() + textHeight + tip_offset - ui->lb_list->height();
    ui->lb_list->setFixedHeight(next_point.y() + textHeight + tip_offset);
    lb->SetCurState(ClickLbState::Selected);
    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height() +
                                      diff_height);
}

void AuthenFriend::slot_remove_friend_label(QString name) {
    qDebug() << "receive close signal";
    _label_point.setX(2);
    _label_point.setY(6);
    auto find_iter = _friend_labels.find(name);
    if (find_iter == _friend_labels.end()) {
        return;
    }
    auto find_key = _friend_label_keys.end();
    for (auto iter = _friend_label_keys.begin();
         iter != _friend_label_keys.end(); iter++) {
        if (*iter == name) {
            find_key = iter;
            break;
        }
    }
    if (find_key != _friend_label_keys.end()) {
        _friend_label_keys.erase(find_key);
    }
    delete find_iter.value();
    _friend_labels.erase(find_iter);
    resetLabels();
    auto find_add = _add_labels.find(name);
    if (find_add == _add_labels.end()) {
        return;
    }
    find_add.value()->ResetNormalState();  // 这个函数没完成
}

void AuthenFriend::slot_change_friend_label_by_tip(QString lbtext,
                                                   ClickLbState state) {
    auto find_iter = _add_labels.find(lbtext);
    if (find_iter == _add_labels.end()) {
        return;
    }
    if (state == ClickLbState::Selected) {
        // 编写添加逻辑
        addLabel(lbtext);
        return;
    }
    if (state == ClickLbState::Normal) {
        // 编写删除逻辑
        slot_remove_friend_label(lbtext);
        return;
    }
}

void AuthenFriend::slot_label_text_change(const QString& text) {
    if (text.isEmpty()) {
        ui->tip_lb->setText("");
        ui->input_tip_widget->hide();
        return;
    }
    auto iter = std::find(_tip_data.begin(), _tip_data.end(), text);
    if (iter == _tip_data.end()) {
        auto new_text = add_prefix + text;
        ui->tip_lb->setText(new_text);
        ui->input_tip_widget->show();
        return;
    }
    ui->tip_lb->setText(text);
    ui->input_tip_widget->show();
}

void AuthenFriend::slot_label_edit_finished() { ui->input_tip_widget->hide(); }

void AuthenFriend::slot_add_friend_label_by_click_tip(QString text) {
    int index = text.indexOf(add_prefix);
    if (index != -1) {
        text = text.mid(index + add_prefix.length());
    }
    addLabel(text);
    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    // 找到了就只需设置状态为选中即可
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }
    // 判断标签展示栏是否有该标签
    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->SetCurState(ClickLbState::Selected);
        return;
    }
    // 标签展示栏也增加一个标签, 并设置绿色选中
    auto* lb = new ClickedLabel(ui->lb_list);
    lb->SetState("normal", "hover", "pressed", "selected_normal",
                 "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &ClickedLabel::clicked, this,
            &AuthenFriend::slot_change_friend_label_by_tip);
    qDebug() << "ui->lb_list->width() is " << ui->lb_list->width();
    qDebug() << "_tip_cur_point.x() is " << _tip_cur_point.x();
    QFontMetrics fontMetrics(lb->font());  // 获取QLabel控件的字体信息
    int textWidth =
        fontMetrics.horizontalAdvance(lb->text());  // 获取文本的宽度
    int textHeight = fontMetrics.height();          // 获取文本的高度
    qDebug() << "textWidth is " << textWidth;
    if (_tip_cur_point.x() + textWidth + tip_offset + 3 >
        ui->lb_list->width()) {
        _tip_cur_point.setX(5);
        _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);
    }
    auto next_point = _tip_cur_point;
    AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);
    _tip_cur_point = next_point;
    int diff_height =
        next_point.y() + textHeight + tip_offset - ui->lb_list->height();
    ui->lb_list->setFixedHeight(next_point.y() + textHeight + tip_offset);
    lb->SetCurState(ClickLbState::Selected);
    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height() +
                                      diff_height);
}

void AuthenFriend::slot_apply_cancel() {
    qDebug() << "Slot Apply Cancel";
    this->hide();
    deleteLater();
}
void AuthenFriend::slot_apply_sure() {
    qDebug() << "Slot Apply Sure called";
    // 添加发送逻辑
    QJsonObject jsonObj;
    auto uid = UserMgr::GetInstance()->GetUid();
    jsonObj["fromuid"] = uid;
    jsonObj["touid"] = _apply_info->_uid;
    QString back_name = "";
    if (ui->back_ed->text().isEmpty()) {
        back_name = ui->back_ed->placeholderText();
    } else {
        back_name = ui->back_ed->text();
    }
    jsonObj["back"] = back_name;

    QJsonDocument doc(jsonObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    // 发送tcp请求给chat server
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_AUTH_FRIEND_REQ,
                                              jsonData);
    this->hide();
    deleteLater();
}
