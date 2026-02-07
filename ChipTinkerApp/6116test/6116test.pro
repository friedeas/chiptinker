#-------------------------------------------------
#
# Project created by QtCreator 2014-08-27T21:25:38
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = 6116test
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
