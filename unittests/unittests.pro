#-------------------------------------------------
#
# Project created by QtCreator 2013-02-26T19:48:35
#
#-------------------------------------------------

include(../ngrt4n.pro)

TARGET = tst_mklshelpertest
CONFIG   += console
CONFIG   -= app_bundle
QMAKE_CXXFLAGS += -std=c++0x
TEMPLATE = app

INCLUDEPATH = ../client/src ../core/src

HEADERS += ../core/src/LsHelper.hpp

SOURCES += tst_mklshelpertest.cpp \
    web_foundation_unittest.cpp
