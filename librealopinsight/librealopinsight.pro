#-------------------------------------------------
#
# Project created by QtCreator 2013-12-20T13:45:19
#
#-------------------------------------------------

QT       -= core gui

TARGET = librealopinsight
TEMPLATE = lib

DEFINES += LIBREALOPINSIGHT_LIBRARY

SOURCES += librealopinsight.cpp

HEADERS += librealopinsight.hpp\
        librealopinsight_global.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
