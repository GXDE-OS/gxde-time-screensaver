#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_m_exitButton_clicked();

private:
    Ui::MainWindow *ui;
    void ChangeInformation();
    void ExitScreenSaver();
    QDateTime firstRunTime;
    QTimer *m_updateTimeTimer;
};
#endif // MAINWINDOW_H
