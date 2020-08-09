# Project settings: ============================================================

QT      += core gui widgets network

CONFIG  += c++14

DEFINES += QT_DEPRECATED_WARNINGS

lessThan( QT_MAJOR_VERSION, 6 ):lessThan( QT_MINOR_VERSION, 14 ) {
    QMAKE_CXXFLAGS += -Wno-deprecated-copy
}

# Output dir: ==================================================================

DESTDIR = $${PWD}/../_distrib/$${QMAKE_HOST.os}-$${QMAKE_HOST.arch}_Qt$${QT_VERSION}

# Project files: ===============================================================

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    network_sender.cpp

HEADERS += \
    mainwindow.h \
    network_sender.h

FORMS += \
    mainwindow.ui

