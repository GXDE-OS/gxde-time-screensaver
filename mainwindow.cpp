#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QTimer>
#include <QDateTime>
#include <cmath>
#include <QDebug>
#include <QWindow>
// http
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
// Json
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QPainter>
#include <QDBusInterface>

#include <QFile>

#include <xcb/xcb.h>
#include <X11/Xlib.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlag(Qt::WindowTransparentForInput, true);
    // 设置定时事件用于更新数据
    m_updateTimeTimer = new QTimer();
    connect(m_updateTimeTimer, &QTimer::timeout, this, &MainWindow::ChangeInformation);
    m_updateTimeTimer->setInterval(500);
    m_updateTimeTimer->start();
    // 记录初始运行时间
    firstRunTime = QDateTime::currentDateTime();
    // 设置屏保背景
    m_background = GetSystemImage();

    //this->setStyleSheet("#MainWindow {border-image: url(:/Background/background.jpg);} * {color: white;}");
    // 添加退出按钮（已废弃）
    /*QLabel *exitButton = new QLabel("<a style='text-decoration: none; color: white;' href='https://www.gfdgdxi.top'>" + tr("退出") + "</a>");
    // 使用超链接实现 QLabel 点击事件
    connect(exitButton, &QLabel::linkActivated, this, [this](){
        this->ExitScreenSaver();
    });
    exitButton->setAlignment(Qt::AlignRight);
    ui->m_timeLayout->addWidget(exitButton, 0, 0);*/
    // 判断系统是否为 GXDE
    if(QFile::exists("/etc/profile.d/gxde.sh")) {
        // 如果是则显示 logo
        ui->m_iconShow->setMinimumWidth(ui->m_iconShow->height()); // 以便可以正确显示图标
    }
    else {
        ui->m_iconShow->setVisible(false);
        ui->m_systemName->setText("©2023～" + QDateTime::currentDateTime().toString("yyyy") + " gfdgd xi");
        ui->m_systemName->setAlignment(Qt::AlignBottom);

    }
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

QImage MainWindow::GetSystemImage()
{
    // 使用 dbus 获取当前壁纸
    QDBusInterface dbus("com.deepin.wm",
                        "/com/deepin/wm",
                        "com.deepin.wm");
    QString backgroundPath = dbus.call("GetCurrentWorkspaceBackground").arguments().at(0).toUrl().path();
    if (QFile::exists(backgroundPath)) {
        return QImage(backgroundPath);
    }
    QImage(":/Background/background.jpg");
}

QSize MainWindow::mapFromHandle(const QSize &handleSize)
{
    qreal ratio = devicePixelRatioF();
    qDebug() << "parent window handle size" << handleSize << "devicePixelRatio" << ratio;

    if (ratio > 0 && ratio != 1.0)
        return handleSize / ratio;
    else
        return handleSize;
}

bool MainWindow::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t *>(message);
        int type = (event->response_type & ~0x80);
        if (XCB_CONFIGURE_NOTIFY == type) {
            xcb_configure_notify_event_t *ce = reinterpret_cast<xcb_configure_notify_event_t *>(event);
            qInfo() << "parent window size changed" << ce->width << ce->height;
            QSize widSize = mapFromHandle(QSize(ce->width, ce->height));
            if (widSize != size()) {
                qInfo() << "update window size:" << widSize;
                resize(widSize);
            }
        } else if (XCB_DESTROY_NOTIFY == type) {
            xcb_destroy_notify_event_t *ce = reinterpret_cast<xcb_destroy_notify_event_t *>(event);
            if (ce->window == Window(this->windowHandle()->winId())) {
                qInfo() << "parent window closed";
                QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
            }
        }
    }
    return false;
}

void MainWindow::ChangePoem()
{
    QUrl url("https://v1.hitokoto.cn/?c=d&c=i&c=k");
    QUrlQuery query;
    query.addQueryItem("type", "DESKDICT");
    query.addQueryItem("c", "d");
    query.addQueryItem("c", "i");
    query.addQueryItem("c", "k");
    query.addQueryItem("num", "4");
    query.addQueryItem("ver", "2.0");
    query.addQueryItem("le", "eng");
    query.addQueryItem("doctype", "json");
    url.setQuery(query.toString(QUrl::FullyEncoded));
    qDebug() << url;
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
        this->ShowPoemText(hitokoto, from, from_who);
        delete m_http;
    });
}

void MainWindow::ShowPoemText(QJsonValue hitokoto, QJsonValue from, QJsonValue from_who)
{
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
}

void MainWindow::GetSentenceOffLine()
{
    // 通过随机数随机选择句子
    srand(time(0)); // 使用时间种子
    int id = rand() % (offLineSentence_count - 1) + 1;
    QJsonObject object = offLineSentence.at(id).toObject();
    QJsonValue hitokoto = object.value("hitokoto");
    QJsonValue from = object.value("from");
    QJsonValue from_who = object.value("from_who");
    this->ShowPoemText(hitokoto, from, from_who);
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

void MainWindow::paintEvent(QPaintEvent *e)
{
    QImage scaleImage = m_background.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPainter painter;
    painter.begin(this);
    int windowWidth = width();
    int windowHeight = height();
    int imageWidth = scaleImage.width();
    int imageHeight = scaleImage.height();
    int x = (windowWidth - imageWidth) / 2;
    int y = (windowHeight - imageHeight) / 2;
    painter.drawImage(x, y, scaleImage);
    painter.setPen(QPen(QColor("#99252525")));
    painter.setBrush(QBrush(QColor("#99252525")));
    painter.drawRect(0, 0, windowWidth, windowHeight);
    painter.end();
    //QMainWindow::paintEvent(e);
}
