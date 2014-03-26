# librealopinsight.pro                                                     #
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 23-03-2014                                                 #
#                                                                          #
# This Software is part of NGRT4N Project (http://ngrt4n.com).             #
#                                                                          #
# This is a free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# The Software is distributed in the hope that it will be useful,          #
# but WITHOUT ANY WARRANTY; without even the implied warranty or           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with the Software.  If not, see <http://www.gnu.org/licenses/>.    #
#--------------------------------------------------------------------------#

QT       += core xml network script

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

TARGET = realopinsight
TEMPLATE = lib
VERSION=1.0.0

win32 {
INCLUDEPATH += $$PWD/../../../ZeroMQ-2.2.0/include
LIBS += -L$$PWD/../../../ZeroMQ-2.2.0/bin -llibzmq-v100-mt
}
unix {
LIBS += -lzmq
}

DEFINES += LIBREALOPINSIGHT_LIBRARY
DEFINES *= QT_USE_QSTRINGBUILDER
DEFINES *= "BUILTIN_USER_PREFIX='\"ngrt4n\"'"
DEFINES *= "APPLICATION_NAME='\"RealOpInsight\"'"
DEFINES *= "PACKAGE_NAME='\"Ultimate\"'"
DEFINES *= "PACKAGE_VERSION='\"$${VERSION}\"'"
DEFINES *= "PACKAGE_URL='\"http://RealOpInsight.com\"'"
DEFINES *= "RELEASE_NAME='\"Everywhere\"'"
DEFINES *= "RELEASE_YEAR='\"2014\"'"
DEFINES *= "BUG_REPORT_EMAIL='\"bugs@realopinsight.com\"'"

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += \
    src/Parser.cpp \
    src/Preferences.cpp \
    src/ZbxHelper.cpp \
    src/JsHelper.cpp \
    src/ZnsHelper.cpp \
    src/Settings.cpp \
    src/utilsClient.cpp \
    src/ZmqSocket.cpp \
    src/LsHelper.cpp \
    src/DashboardBase.cpp

HEADERS += \
    src/Base.hpp \
    src/Parser.hpp \
    src/Preferences.hpp \
    src/ZbxHelper.hpp \
    src/JsHelper.hpp \
    src/ZnsHelper.hpp \
    src/Settings.hpp \
    src/utilsClient.hpp \
    src/ZmqSocket.hpp \
    src/LsHelper.hpp \
    src/DashboardBase.hpp \
    librealopinsight_global.h \
    src/global.hpp
