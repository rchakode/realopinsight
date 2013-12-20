#-------------------------------------------------
#
# Project created by QtCreator 2013-12-20T13:45:19
#
#-------------------------------------------------

QT       += core gui xml network script


TARGET = realopinsight
TEMPLATE = lib
VERSION=1.0.0

DEFINES += LIBREALOPINSIGHT_LIBRARY


DEFINES *= QT_USE_QSTRINGBUILDER
DEFINES *= "BUILTIN_USER_PREFIX='\"ngrt4n\"'"
DEFINES *= "APPLICATION_NAME='\"RealOpInsight\"'"
DEFINES *= "PACKAGE_NAME='\"Ultimate\"'"
DEFINES *= "PACKAGE_VERSION='\"$${VERSION_}\"'"
DEFINES *= "PACKAGE_URL='\"http://RealOpInsight.com\"'"
DEFINES *= "RELEASE_NAME='\"Everywhere\"'"
DEFINES *= "RELEASE_YEAR='\"2014\"'"

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += \
    src/MonitorBroker.cpp \
    src/Parser.cpp \
    src/Preferences.cpp \
    src/StatsLegend.cpp \
    src/ZbxHelper.cpp \
    src/JsHelper.cpp \
    src/ZnsHelper.cpp \
    src/Settings.cpp \
    src/utilsClient.cpp \
    src/utilsCore.cpp \
    src/ZmqSocket.cpp \
    src/LsHelper.cpp \
    src/DashboardBase.cpp

HEADERS += src/ns.hpp \
    src/MonitorBroker.hpp \
    src/Base.hpp \
    src/Parser.hpp \
    src/Preferences.hpp \
    src/StatsLegend.hpp \
    src/ZbxHelper.hpp \
    src/JsHelper.hpp \
    src/ZnsHelper.hpp \
    src/Settings.hpp \
    src/utilsClient.hpp \
    src/Chart.hpp \
    src/ZmqSocket.hpp \
    src/LsHelper.hpp \
    src/DashboardBase.hpp \
    librealopinsight_global.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

