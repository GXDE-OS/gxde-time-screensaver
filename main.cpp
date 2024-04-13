#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QStringList>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile file("/tmp/a.txt");
    file.open(QFile::WriteOnly);
    for(int i = 0; i <= argc; i++){
        file.write(argv[i]);
        file.write(" ");
    }
    file.close();
    MainWindow w;
    w.show();
    return a.exec();
}
