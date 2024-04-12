#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QTimer>
#include <QDateTime>
#include <cmath>

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
    this->setAttribute(Qt::WA_TransparentForMouseEvents, true); // 鼠标事件穿透
    // 此外将鼠标事件穿透到标准屏幕保护程序，程序收到任何移动事件后将自动还原到此前状态
    // 如果未穿透，将可用作普通程序使用，您可以用来定义为某种仅支持鼠标操作的在线资源页面
    //this->setStyleSheet("background:transparent");
    // 设置全屏
    QDesktopWidget *desktop = QApplication::desktop();
    this->resize(desktop->width(), desktop->height());
    // 设置定时事件用于更新数据
    QTimer *timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &MainWindow::ChangeInformation);
    timer->setInterval(500);
    timer->start();
    // 记录初始运行时间
    firstRunTime = QDateTime::currentDateTime();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ChangeInformation()
{
    QDateTime time = QDateTime::currentDateTime();
    // 显示当前时间
    ui->m_time->setText("<h1>" + time.toString("hh:mm:ss") + "</h1>");
    // 获取相差秒数
    int second = firstRunTime.secsTo(time);
    int minute = second / 60;
    // 计算位数
    if(minute >= 100) {
        // 大于 100 才进行计算，否则均默认 2 位
        // 使用 lg(x) 进行计算
        ui->m_minTime->setDigitCount(log10(minute) + 1);
    }
    ui->m_minTime->display(minute);
    ui->m_secondTime->display(second % 60);
}


void MainWindow::on_m_exitButton_clicked()
{
    this->close();
}

