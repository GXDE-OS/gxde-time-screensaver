#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QTimer>
#include <QJsonArray>

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
    void ChangePoem();
    void ExitScreenSaver();
    QJsonObject GetSentenceOffLine();
    QJsonArray offLineSentence;
    int offLineSentence_count;
    QDateTime firstRunTime;
    QTimer *m_updateTimeTimer;
    QTimer *m_updateSentencesTimer;
};
#endif // MAINWINDOW_H

