#-------------------------------------------------
#
# Project created by QtCreator 2014-07-27T19:48:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ChipTinkerApp
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    modelselector.cpp \
    icvisualizer.cpp \
    ../testsheet.cpp \
    ../powertable.cpp \
    ../devicedriver.cpp \
    testlog.cpp \
    powervisualizer.cpp \
    configwindow.cpp \
    lowleveltest.cpp \
    sheeteditor.cpp \
    romdumper.cpp \
    ../romlist.cpp \
    ../romalgorithms.cpp \
    identifier.cpp

HEADERS  += mainwindow.h \
    modelselector.h \
    icvisualizer.h \
    ../testsheet.h \
    ../powertable.h \
    ../devicedriver.h \
    testlog.h \
    powervisualizer.h \
    configwindow.h \
    lowleveltest.h \
    sheeteditor.h \
    romdumper.h \
    ../romlist.h \
    ../romalgorithms.h \
    identifier.h

FORMS    += mainwindow.ui \
    modelselector.ui \
    powervisualizer.ui \
    configwindow.ui \
    lowleveltest.ui \
    sheeteditor.ui \
    romdumper.ui \
    identifier.ui

lessThan(QT_MAJOR_VERSION, 5) {
    CONFIG += serialport
} else {
    QT +=serialport
}


RESOURCES += \
    resources.qrc

TRANSLATIONS = _QtICTester_pl_PL.ts

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}

updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link

QMAKE_EXTRA_COMPILERS += updateqm

PRE_TARGETDEPS += compiler_updateqm_make_all

DISTFILES +=
