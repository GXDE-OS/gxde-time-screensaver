#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QTimer>
#include <QDateTime>
#include <cmath>
#include <QDebug>

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
    qDebug() << "a";
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

