#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include <QLabel>
#include <QMouseEvent>
#include <QObject>
#include <QPaintEvent>

#include "global.h"

class StateWidget : public QWidget {
    Q_OBJECT
   public:
    explicit StateWidget(QWidget *parent = nullptr);
    void SetState(QString normal = "", QString hover = "", QString press = "",
                  QString select = "", QString select_hover = "",
                  QString select_press = "");
    ClickLbState GetCurState();
    void ClearState();
    void SetSelected(bool bselected);
    void AddRedPoint();
    void ShowReadPoint(bool show = true);

   protected:
    // 为了让qss生效
    void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void enterEvent(QEnterEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;

   private:
    QString _normal;
    QString _normal_hover;
    QString _normal_press;

    QString _selected;
    QString _selected_hover;
    QString _selected_press;

    ClickLbState _curState;
    QLabel *_red_point;

   signals:
    void clicked(void);

   public slots:
};

#endif  // STATEWIDGET_H
