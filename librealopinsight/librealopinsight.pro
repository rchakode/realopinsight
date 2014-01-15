# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2013 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-12-2013                                                 #
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

QT       += core gui xml network script

TARGET = realopinsight
TEMPLATE = lib
LIBS += -lzmq
VERSION=1.0.0

DEFINES += LIBREALOPINSIGHT_LIBRARY
DEFINES *= QT_USE_QSTRINGBUILDER
DEFINES *= "BUILTIN_USER_PREFIX='\"ngrt4n\"'"
DEFINES *= "APPLICATION_NAME='\"RealOpInsight\"'"
DEFINES *= "PACKAGE_NAME='\"Ultimate\"'"
DEFINES *= "PACKAGE_VERSION='\"$${VERSION}\"'"
DEFINES *= "PACKAGE_URL='\"http://RealOpInsight.com\"'"
DEFINES *= "RELEASE_NAME='\"Everywhere\"'"
DEFINES *= "RELEASE_YEAR='\"2014\"'"
DEFINES *= "BUG_REPORT_EMAIL='\"bugs@ngrt4n.com\"'"

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

