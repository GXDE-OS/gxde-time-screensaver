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
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
    ~MainWindow();

private slots:
    void on_m_exitButton_clicked();

private:
    QImage GetSystemImage();
    Ui::MainWindow *ui;
    void ChangeInformation();
    void ChangePoem();
    void ExitScreenSaver();
    void ShowPoemText(QJsonValue hitokoto, QJsonValue from, QJsonValue from_who);
    void GetSentenceOffLine();
    QJsonArray offLineSentence;
    int offLineSentence_count;
    QDateTime firstRunTime;
    QTimer *m_updateTimeTimer;
    QTimer *m_updateSentencesTimer;
    QImage m_background;
    QSize mapFromHandle(const QSize &handleSize);

    void paintEvent(QPaintEvent*);
};
#endif // MAINWINDOW_H

