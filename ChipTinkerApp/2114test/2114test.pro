#-------------------------------------------------
#
# Project created by QtCreator 2014-08-24T18:22:31
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = 2114test
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


HEADERS +=
