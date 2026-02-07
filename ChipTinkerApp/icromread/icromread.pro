#-------------------------------------------------
#
# Project created by QtCreator 2015-07-09T18:41:39
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = icromread
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    ../powertable.cpp \
    ../testsheet.cpp \
    ../romalgorithms.cpp \
    ../romlist.cpp \
    ../devicedriver.cpp

HEADERS += \
    ../powertable.h \
    ../testsheet.h \
    ../romalgorithms.h \
    ../romlist.h \
    ../devicedriver.h


lessThan(QT_MAJOR_VERSION, 5) {
    CONFIG += serialport
} else {
    QT +=serialport
}
