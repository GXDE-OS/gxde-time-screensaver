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
#include <QScreen>

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

    ui->label->setHidden(true);
    ui->label_2->setHidden(true);
    ui->label_3->setHidden(true);
    ui->m_hourTime->setHidden(true);
    ui->m_minTime->setHidden(true);
    ui->m_secondTime->setHidden(true);

    QPixmap pixmap = *ui->m_iconShow->pixmap();
    pixmap.setDevicePixelRatio(QGuiApplication::primaryScreen()->devicePixelRatio());
    ui->m_iconShow->setPixmap(pixmap);

    ui->m_iconShow->setVisible(false);

    //this->setStyleSheet("#MainWindow {border-image: url(:/Background/background.jpg);} * {color: white;}");
    // 添加退出按钮（已废弃）
    /*QLabel *exitButton = new QLabel("<a style='text-decoration: none; color: white;' href='https://www.gfdgdxi.top'>" + tr("退出") + "</a>");
    // 使用超链接实现 QLabel 点击事件
    connect(exitButton, &QLabel::linkActivated, this, [this](){
        this->ExitScreenSaver();
    });
    exitButton->setAlignment(Qt::AlignRight);
    ui->m_timeLayout->addWidget(exitButton, 0, 0);*/
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
    // 修改时间默认字体大小
    QFont timeFont = ui->m_time->font();
    timeFont.setPointSize(timeFont.pointSize() * 2);
    ui->m_time->setFont(timeFont);
    //修改句子默认字体大小
    QFont senFont = ui->m_poem->font();
    senFont.setPointSize(timeFont.pointSize() * 1.1);
    ui->m_poem->setFont(senFont);
}

QImage MainWindow::GetSystemImage()
{
    // 使用 dbus 获取当前壁纸
    QDBusInterface dbus("com.deepin.wm",
                        "/com/deepin/wm",
                        "com.deepin.wm");
    QString backgroundPath = dbus.call("GetCurrentWorkspaceBackground").arguments().at(0).toUrl().path();
    // 如果能正常读取到数据，则优先使用读取到的
    if (QFile::exists(backgroundPath)) {
        return QImage(backgroundPath);
    }
    // 反之使用默认壁纸
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
    // 使用离线数据
    GetSentenceOffLine();
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
    qreal ratio = QGuiApplication::primaryScreen()->devicePixelRatio();
    QSize backgroundSize = size() * ratio;
    QImage scaleImage = m_background.scaled(backgroundSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    scaleImage.setDevicePixelRatio(ratio);
    QPainter painter;
    painter.begin(this);
    int windowWidth = width() * ratio;
    int windowHeight = height()* ratio;
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
