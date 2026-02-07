TEMPLATE = subdirs

SUBDIRS += \
    ictestcon \
    ChipTinkerApp \
    2114test \
    6116test \
    icromread \
    2102test \
    6810test \
    93415test \
    2111test

lessThan(QT_MAJOR_VERSION, 5) {
    CONFIG += serialport
} else {
    QT +=serialport
}


DISTFILES += \
    CHANGES.TXT
