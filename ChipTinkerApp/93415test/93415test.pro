QT += core
QT -= gui

CONFIG += c++11

TARGET = 93415test
CONFIG += console
CONFIG -= app_bundle

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
