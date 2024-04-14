#include "mainwindow.h"
#include "commandlinemanager.h"

#include <QApplication>
#include <QFile>
#include <QStringList>
#include <QWindow>
#include <QDebug>

#include <X11/Xlib.h>

int main(int argc, char *argv[])
{
    auto envType = qEnvironmentVariable("XDG_SESSION_TYPE");
    if (envType.contains("wayland")) {
        qInfo() << QDateTime::currentDateTime().toString() << "notes:change wayland to xcb for QT_QPA_PLATFORM.";
        qputenv("QT_QPA_PLATFORM", "xcb");
    }

    QApplication a(argc, argv);
    CommandLineManager::instance()->process(a.arguments());
    if (CommandLineManager::instance()->isSet("window-id")) {
        QString windowId = CommandLineManager::instance()->value("window-id");
        WId windowHwnd = WId(windowId.toULongLong());
        QWindow *window = QWindow::fromWinId(windowHwnd);

        if (!window) {
            qDebug() << "Error:not found QWindow by window id:" << windowId;
            return -1;
        }
        MainWindow w;
        w.setProperty("_q_embedded_native_parent_handle", QVariant(windowHwnd));
        w.winId();
        w.windowHandle()->setParent(window);

        Display *display = XOpenDisplay(nullptr);
        if (!display) {
            qWarning() << "can not connect xservice.";
            return -1;
        }

        Window windowXID(windowHwnd);
        XWindowAttributes attr;
        XGetWindowAttributes(display, windowXID, &attr);

        w.setGeometry(QRect(0, 0, attr.width, attr.height));

        w.show();
        return a.exec();
    }
    else {
        MainWindow w;
        w.show();
        return a.exec();
    }
}
