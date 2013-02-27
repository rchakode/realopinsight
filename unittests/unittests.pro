#-------------------------------------------------
#
# Project created by QtCreator 2013-02-26T19:48:35
#
#-------------------------------------------------

QT       += core gui xml svg webkit network script testlib

#QT       -= gui

TARGET = tst_mklshelpertest
CONFIG   += console
CONFIG   -= app_bundle
QMAKE_CXXFLAGS += -std=c++0x
TEMPLATE = app

INCLUDEPATH = ../include ../include/client ../include/core

HEADERS += ../include/client/MkLsHelper.hpp

SOURCES += ../src/client/MkLsHelper.cpp tst_mklshelpertest.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"
DEFINES *= QT_USE_QSTRINGBUILDER
DEFINES *= "BUILTIN_USER_PREFIX='\"ngrt4n\"'"
DEFINES *= "APPLICATION_NAME='\"RealOpInsight\"'"
DEFINES *= "PACKAGE_NAME='\"\"'"
DEFINES *= "PACKAGE_VERSION='\"$${VERSION_}\"'"
DEFINES *= "PACKAGE_URL='\"http://RealOpInsight.com\"'"
DEFINES *= "RELEASE_INFO='\"02/2013\"'"
DEFINES *= "RELEASE_NAME='\"Lucia\"'"

include(../QsLog/QsLog.pri)
