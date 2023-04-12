QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Issue on Raspberry Pi OS that a double click does not start the executable as it thinks it's a shared library :-(
equals(QMAKE_CXX, g++): QMAKE_LFLAGS += -no-pie

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

# QtQuick forms look quite different on Windows and on Linux (Ubuntu), so we hold separate form files
win32: FORMS += mainwindow_win.ui
else: FORMS += mainwindow_fusion.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
