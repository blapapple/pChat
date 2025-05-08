#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
/*************************************************************************
 * @file                mainwindow.h
 * @brief               XXXX Function
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

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
