#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QTimer>
#include <QDateTime>
#include <cmath>
#include <QDebug>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
// Json
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QFile>

#include <random>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 隐藏标题栏
    this->setWindowFlags(Qt::Tool                       // 此属性无最大最小化按键
                         |Qt::WindowStaysOnTopHint       // 窗口保持在顶部提示
                         |Qt::FramelessWindowHint        // 无框窗口提示
                                                                                    |Qt::X11BypassWindowManagerHint // 不被X11窗口管理器管理
                                                         // 一但运行，除非被 kill，将无法被控制
                                                         // 也是作为屏保正常运行时的必须属性
     );
    //this->setAttribute(Qt::WA_TranslucentBackground);           // 半透明的背景
    //this->setAttribute(Qt::WA_TransparentForMouseEvents, true); // 鼠标事件穿透
    // 此外将鼠标事件穿透到标准屏幕保护程序，程序收到任何移动事件后将自动还原到此前状态
    // 如果未穿透，将可用作普通程序使用，您可以用来定义为某种仅支持鼠标操作的在线资源页面
    //this->setStyleSheet("background:transparent");

    // 设置全屏
    QDesktopWidget *desktop = QApplication::desktop();
    this->resize(desktop->width(), desktop->height());
    // 设置定时事件用于更新数据
    m_updateTimeTimer = new QTimer();
    connect(m_updateTimeTimer, &QTimer::timeout, this, &MainWindow::ChangeInformation);
    m_updateTimeTimer->setInterval(500);
    m_updateTimeTimer->start();
    // 记录初始运行时间
    firstRunTime = QDateTime::currentDateTime();
    // 设置屏保背景
    this->setStyleSheet("#MainWindow {border-image: url(:/Background/background.jpg);} * {color: white;}");
    // 添加退出按钮
    QLabel *exitButton = new QLabel("<a style='text-decoration: none; color: white;' href='https://www.gfdgdxi.top'>" + tr("退出") + "</a>");
    // 使用超链接实现 QLabel 点击事件
    connect(exitButton, &QLabel::linkActivated, this, [this](){
        this->ExitScreenSaver();
    });
    exitButton->setAlignment(Qt::AlignRight);
    ui->m_timeLayout->addWidget(exitButton, 0, 0);
    ui->m_iconShow->setMinimumWidth(ui->m_iconShow->height()); // 以便可以正确显示图标
    // 句子更新 QTimer
    m_updateSentencesTimer = new QTimer();
    m_updateSentencesTimer->setInterval(60 * 1000);
    connect(m_updateSentencesTimer, &QTimer::timeout, this, &MainWindow::ChangePoem);
    m_updateSentencesTimer->start();
    // 读取离线词库
    QFile sentence(":/Poem/poem.json");
    sentence.open(QFile::ReadOnly);
    offLineSentence = QJsonDocument::fromJson(sentence.readAll()).array();
    offLineSentence_count = offLineSentence.count(); // 提前计算数据以减少损耗
    sentence.close();
    ChangePoem();
}


void MainWindow::ChangePoem()
{
    QUrl url("https://v1.hitokoto.cn");
    QUrlQuery query;
    query.addQueryItem("type", "DESKDICT");
    query.addQueryItem("client", "GXDE");
    query.addQueryItem("keyfrom", "GXDE");
    query.addQueryItem("num", "4");
    query.addQueryItem("ver", "2.0");
    query.addQueryItem("le", "eng");
    query.addQueryItem("doctype", "json");
    url.setQuery(query.toString(QUrl::FullyEncoded));
    QNetworkRequest request(url);
    QNetworkAccessManager *m_http = new QNetworkAccessManager(this);
    QNetworkReply *reply = m_http->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, m_http](){
        QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            GetSentenceOffLine(); // 如果无法访问则使用离线数据库
            return;
        }
        QByteArray data = reply->readAll();
        qDebug() << data;
        // 解析数据
        // 数据例子：
        /*
        {
            "id": 6256,
            "uuid": "b2f3834f-5f52-4b62-bb3c-f9fb98901419",
            "hitokoto": "我们在努力扩大自己，以靠近，以触及我们自身以外的世界。",
            "type": "k",
            "from": "豪尔赫·路易斯·博尔赫斯",
            "from_who": "博尔赫斯谈话录",
            "creator": "Irony",
            "creator_uid": 4464,
            "reviewer": 1044,
            "commit_from": "web",
            "created_at": "1592126239",
            "length": 27
        }*/
        QJsonDocument document = QJsonDocument::fromJson(data);
        QJsonObject object = document.object();
        QJsonValue hitokoto = object.value("hitokoto");
        QJsonValue from = object.value("from");
        QJsonValue from_who = object.value("from_who");
        // 显示文本
        QString showText = "<p>" + hitokoto.toString() + "</p><p align='right'>";
        QString who = "";
        if (!from.isNull()) {
            who += "《" + from.toString() + "》";
        }
        if (!from_who.isNull()) {
            who += from_who.toString();
        }
        if (who.length() > 0) {
            // 有内容，添加破折号
            who = "——" + who;
        }
        showText += who + "</p>";
        ui->m_poem->setText(showText);
        delete m_http;
    });
}

QJsonObject MainWindow::GetSentenceOffLine()
{
    std::default_random_engine e;
    std::uniform_int_distribution u(0, offLineSentence_count);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ChangeInformation()
{
    QDateTime time = QDateTime::currentDateTime();
    // 显示当前时间
    ui->m_time->setText("<h1>" + time.toString("hh:mm:ss") + "</h1><p>" + time.toString("yyyy/MM/dd dddd") + "</p>");
    // 获取相差秒数
    int second = firstRunTime.secsTo(time);
    int minute = second / 60 % 60;
    int hour = second / 60 / 60;
    // 计算位数
    if(hour >= 100) {
        // 大于 100 才进行计算，否则均默认 2 位
        // 使用 lg(x) 进行计算
        ui->m_hourTime->setDigitCount(log10(hour) + 1);
    }
    ui->m_hourTime->display(hour);
    ui->m_minTime->display(minute);
    ui->m_secondTime->display(second % 60);
}

void MainWindow::ExitScreenSaver()
{
    // 关闭 QTimer
    this->m_updateTimeTimer->stop();
    this->close();
    exit(0);
}


void MainWindow::on_m_exitButton_clicked()
{
    ExitScreenSaver();
}

