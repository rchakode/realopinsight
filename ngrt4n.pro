# ngrt4n.pro
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 24-03-2014                                                 #
#                                                                          #
# This file is part of RealOpInsight (http://RealOpInsight.com) authored   #
# by Rodrigue Chakode <rodrigue.chakode@gmail.com>                         #
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

QT	+= core xml network script

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets webkitwidgets
}

CONFIG += no_keywords
TEMPLATE = app
REALOPINSIGHT_CORE_VERSION=3.0.2
VERSION = "-$${REALOPINSIGHT_CORE_VERSION}"

win32 {
DEFINES *= WIN32
DEFINES *= WIN32_LEAN_AND_MEAN
INCLUDEPATH += $$PWD/../../../ZeroMQ-2.2.0/include
LIBS += -lws2_32 -L$$PWD/../../../ZeroMQ-2.2.0/bin -llibzmq-v100-mt
}
unix {
LIBS += -lzmq
}

DEFINES *= BOOST_TT_HAS_OPERATOR_HPP_INCLUDED

OBJECTS_DIR = generated/obj
MOC_DIR = generated/moc
RCC_DIR = generated/rcc
QMAKE_CXXFLAGS += -std=c++0x -Werror -Wno-unused-variable  -Wno-unused-parameter -Wno-unused-local-typedefs

DISTFILES += README.md \
    install-sh \
    install-sh.obs \
    INSTALL \
    client/src/ngrt4n-*.cpp \
    LICENSE \
    ChangeLog \
    AUTHORS \
    images/*.png \
    images/built-in/*.png \
    examples/{*.ngrt4n.xml,*.dat} \
    doc/man/*.gz \
    i18n/ngrt4n_*.qm

TRANSLATIONS += i18n/ngrt4n_fr.ts

CODECFORSRC = UTF-8

HEADERS += \
    core/src/Base.hpp \
    core/src/Parser.hpp \
    core/src/Preferences.hpp \
    core/src/ZbxHelper.hpp \
    core/src/ZnsHelper.hpp \
    core/src/Settings.hpp \
    core/src/ZmqSocket.hpp \
    core/src/LsHelper.hpp \
    core/src/DashboardBase.hpp \
    core/src/global.hpp \
    core/src/utilsCore.hpp \
    core/src/ChartBase.hpp \
    core/src/JsonHelper.hpp \
    core/src/RawSocket.hpp


SOURCES += \
    core/src/Parser.cpp \
    core/src/Preferences.cpp \
    core/src/ZbxHelper.cpp \
    core/src/ZnsHelper.cpp \
    core/src/Settings.cpp \
    core/src/ZmqSocket.cpp \
    core/src/LsHelper.cpp \
    core/src/DashboardBase.cpp \
    core/src/utilsCore.cpp \
    core/src/ChartBase.cpp \
    core/src/JsonHelper.cpp \
    core/src/RawSocket.cpp

gui-base {
QT += svg gui webkit
PACKAGE_VERSION=3.0.3
HEADERS	+= client/src/Auth.hpp \
    client/src/StatsLegend.hpp \
    client/src/GraphView.hpp \
    client/src/PieChart.hpp \
    client/src/ServiceEditor.hpp \
    client/src/SvConfigCreator.hpp \
    client/src/GuiDashboard.hpp \
    client/src/MainWindow.hpp \
    client/src/SvNavigatorTree.hpp \
    client/src/WebKit.hpp \
    client/src/MsgConsole.hpp \
    client/src/GuiPreferences.hpp \
    client/src/GuiUtils.hpp \
    client/src/GuiDialogForms.hpp

SOURCES	+= client/src/Auth.cpp \
    client/src/GuiUtils.cpp \
    client/src/StatsLegend.cpp \
    client/src/GraphView.cpp \
    client/src/PieChart.cpp \
    client/src/ServiceEditor.cpp \
    client/src/SvConfigCreator.cpp \
    client/src/GuiDashboard.cpp \
    client/src/MainWindow.cpp \
    client/src/SvNavigatorTree.cpp \
    client/src/WebKit.cpp \
    client/src/GuiPreferences.cpp \
    client/src/MsgConsole.cpp \
    client/src/GuiDialogForms.cpp
}

dist {
SOURCES	+=
TARGET = realopinsight-workstation
}

manager {
SOURCES	+= client/src/ngrt4n-manager.cpp
TARGET = realopinsight-manager
}

oc {
SOURCES	+= client/src/ngrt4n-oc.cpp
TARGET = realopinsight-oc
}

editor {
SOURCES	+= client/src/ngrt4n-editor.cpp
TARGET = realopinsight-editor
}

web-base {
PACKAGE_VERSION=3.0.2
DEFINES *= REALOPINSIGHT_WEB
DEFINES *= WT_NO_SLOT_MACROS

LIBS += -lwt -lwtdbo -lwtdbosqlite3 \
        -lboost_signals -lboost_program_options -lboost_system \
        -lboost_thread -lboost_regex -lboost_signals \
        -lboost_filesystem -lboost_date_time

INCLUDEPATH += wt/src \
               wt/dbo \
               wt/extlibs/wtwithqt

HEADERS	+= wt/src/WebDashboard.hpp \
    wt/src/WebMsgConsole.hpp \
    wt/src/WebMap.hpp \
    wt/src/WebTree.hpp \
    wt/src/WebPieChart.hpp \
    wt/src/WebMainUI.hpp \
    wt/dbo/DbSession.hpp \
    wt/dbo/UserMngtUI.hpp \
    wt/dbo/DbObjects.hpp \
    wt/dbo/ViewMgnt.hpp \
    wt/src/WebUtils.hpp \
    wt/src/AuthManager.hpp \
    wt/src/WebPreferences.hpp \
    wt/extlibs/wtwithqt/DispatchThread.h \
    wt/extlibs/wtwithqt/WQApplication

SOURCES	+= wt/src/WebDashboard.cpp \
    wt/src/WebMsgConsole.cpp \
    wt/src/ngrt4n-web.cpp \
    wt/src/WebMap.cpp \
    wt/src/WebTree.cpp \
    wt/src/WebPieChart.cpp \
    wt/src/WebMainUI.cpp \
    wt/dbo/DbSession.cpp \
    wt/dbo/UserMngtUI.cpp\
    wt/dbo/ViewMgnt.cpp \
    wt/src/WebUtils.cpp \
    wt/src/AuthManager.cpp \
    wt/src/WebPreferences.cpp \
    wt/extlibs/wtwithqt/DispatchThread.C \
    wt/extlibs/wtwithqt/WQApplication.C
}


webd {
  TARGET = realopinsightd
  LIBS += -lwthttp
}

web-fcgi {
  DEFINES *= REALOPINSIGHT_WEB_FASTCGI
  TARGET = realopinsight.fcgi
  LIBS += -lwtfcgi
}

dflag {
TARGET.path=$$(INSTALL_PREFIX)/bin
MAN.path =$$(INSTALL_PREFIX)/share/man/man1
} else {
TARGET.path=/usr/local/bin
MAN.path = /usr/share/man/man1
}

TARGET.files = $${TARGET}
MAN.files = doc/man/realopinsight-manager.1.gz doc/man/realopinsight-oc.1.gz doc/man/realopinsight-editor.1.gz
INSTALLS += TARGET MAN


INCLUDEPATH += core/src/
RESOURCES += ngrt4n.qrc

DEFINES *= "REALOPINSIGHT_BUILD_DATE=\"1400095511\""
DEFINES *= QT_USE_QSTRINGBUILDER
DEFINES *= "REALOPINSIGHT_BUILTIN_USER_PREFIX='\"ngrt4n\"'"
DEFINES *= "REALOPINSIGHT_APPLICATION_NAME='\"RealOpInsight\"'"
DEFINES *= "REALOPINSIGHT_CORE_VERSION='\"$${REALOPINSIGHT_CORE_VERSION}\"'"
DEFINES *= "REALOPINSIGHT_PACKAGE_VERSION='\"$${PACKAGE_VERSION}\"'"
DEFINES *= "REALOPINSIGHT_PACKAGE_URL='\"http://RealOpInsight.com\"'"
DEFINES *= "REALOPINSIGHT_RELEASE_NAME='\"Eliana\"'"
DEFINES *= "REALOPINSIGHT_RELEASE_YEAR='\"2014\"'"
DEFINES *= "REALOPINSIGHT_BUG_REPORT_EMAIL='\"bugs@realopinsight.com\"'"
DEFINES *= "REALOPINSIGHT_GET_HELP_URL='\"http://docs.realopinsight.com/\"'"
DEFINES *= "REALOPINSIGHT_WWW_ROOT='\"REALOPINSIGHT_WWW_ROOT_VALUE\"'"
