#-------------------------------------------------
#
# Project created by QtCreator 2014-07-26T23:55:34
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = ictestcon
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    ../powertable.cpp \
    ../testsheet.cpp \
    ../devicedriver.cpp

HEADERS += \
    ../powertable.h \
    ../testsheet.h \
    ../devicedriver.h


lessThan(QT_MAJOR_VERSION, 5) {
    CONFIG += serialport
} else {
    QT +=serialport
}
