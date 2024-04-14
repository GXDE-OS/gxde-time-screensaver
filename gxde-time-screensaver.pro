QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 link_pkgconfig

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

PKGCONFIG += x11

SOURCES += \
    commandlinemanager.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    commandlinemanager.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

target.path = $${PREFIX}/lib/deepin-screensaver/modules/
cover.path = $${PREFIX}/lib/deepin-screensaver/modules/cover/
cover.files = $$PWD/cover/*.png
INSTALLS += target cover

RESOURCES += \
    Resource.qrc
