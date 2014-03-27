# ngrt4n.pro
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 24-12-2014                                                 #
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

QT	+= core xml network script

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets webkitwidgets
}

CONFIG += no_keywords
TEMPLATE = app
BASE_VERSION=2.4.2
VERSION = "-$${BASE_VERSION}"


win32 {
INCLUDEPATH += $$PWD/../../../ZeroMQ-2.2.0/include
LIBS += -L$$PWD/../../../ZeroMQ-2.2.0/bin -llibzmq-v100-mt
}
unix {
LIBS += -lzmq
}

INCLUDEPATH += core/src/
RESOURCES += ngrt4n.qrc
DEFINES *= QT_USE_QSTRINGBUILDER
DEFINES *= "BUILTIN_USER_PREFIX='\"ngrt4n\"'"
DEFINES *= "APPLICATION_NAME='\"RealOpInsight\"'"
DEFINES *= "PACKAGE_NAME='\"Ultimate\"'"
DEFINES *= "PACKAGE_VERSION='\"$${PACKAGE_VERSION}\"'"
DEFINES *= "PACKAGE_URL='\"http://RealOpInsight.com\"'"
DEFINES *= "RELEASE_NAME='\"Everywhere\"'"
DEFINES *= "RELEASE_YEAR='\"2014\"'"
DEFINES *= "BUG_REPORT_EMAIL='\"bugs@realopinsight.com\"'"
DEFINES *= "GET_HELP_URL='\"http://realopinsight.com/library/getting-started\"'"

DEFINES *=BOOST_TT_HAS_OPERATOR_HPP_INCLUDED

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
QMAKE_CXXFLAGS += -std=c++0x -Werror -Wno-unused-variable  -Wno-unused-parameter -Wno-unused-local-typedefs

DISTFILES += README \
    INSTALL \
    COPYING \
    NEWS \
    ChangeLog \
    AUTHORS \
    install-sh \
    images/*.png \
    images/built-in/*.png \
    examples/{*.ngrt4n.xml,*.dat} \
    client/src/ngrt4n-*.cpp \
    doc/man/*.gz \
    i18n/ngrt4n_*.qm

TRANSLATIONS += i18n/ngrt4n_fr.ts

CODECFORSRC = UTF-8

HEADERS += \
    core/src/Base.hpp \
    core/src/Parser.hpp \
    core/src/Preferences.hpp \
    core/src/ZbxHelper.hpp \
    core/src/JsHelper.hpp \
    core/src/ZnsHelper.hpp \
    core/src/Settings.hpp \
    core/src/ZmqSocket.hpp \
    core/src/LsHelper.hpp \
    core/src/DashboardBase.hpp \
    core/src/global.hpp \
    core/src/utilsCore.hpp


SOURCES += \
    core/src/Parser.cpp \
    core/src/Preferences.cpp \
    core/src/ZbxHelper.cpp \
    core/src/JsHelper.cpp \
    core/src/ZnsHelper.cpp \
    core/src/Settings.cpp \
    core/src/ZmqSocket.cpp \
    core/src/LsHelper.cpp \
    core/src/DashboardBase.cpp \
    core/src/utilsCore.cpp

gui-base {
QT += svg gui webkit
PACKAGE_VERSION = "$${BASE_VERSION}-SE"
HEADERS	+= client/src/Auth.hpp \
    client/src/StatsLegend.hpp \
    client/src/GraphView.hpp \
    client/src/PieChart.hpp \
    client/src/PieChartItem.hpp \
    client/src/ServiceEditor.hpp \
    client/src/StatsLegend.hpp \
    client/src/SvConfigCreator.hpp \
    client/src/GuiDashboard.hpp \
    client/src/MainWindow.hpp \
    client/src/SvNavigatorTree.hpp \
    client/src/WebKit.hpp \
    client/src/Chart.hpp \
    client/src/MsgConsole.hpp \
    client/src/GuiPreferences.hpp \
    client/src/GuiUtils.hpp

SOURCES	+= client/src/Auth.cpp \
    client/src/GuiUtils.cpp \
    client/src/StatsLegend.cpp \
    client/src/GraphView.cpp \
    client/src/PieChart.cpp \
    client/src/PieChartItem.cpp \
    client/src/ServiceEditor.cpp \
    client/src/SvConfigCreator.cpp \
    client/src/GuiDashboard.cpp \
    client/src/MainWindow.cpp \
    client/src/SvNavigatorTree.cpp \
    client/src/WebKit.cpp \
    client/src/Chart.cpp \
    client/src/GuiPreferences.cpp \
    client/src/MsgConsole.cpp
}

dist {
SOURCES	+=
TARGET = RealOpInsight
}

manager {
SOURCES	+= client/src/ngrt4n-manager.cpp
TARGET = ngrt4n-manager
}

oc {
SOURCES	+= client/src/ngrt4n-oc.cpp
TARGET = ngrt4n-oc
}

editor {
SOURCES	+= client/src/ngrt4n-editor.cpp
TARGET = ngrt4n-editor
}

web {
PACKAGE_VERSION = "$${BASE_VERSION}-UE"
TARGET = realopinsight.fcgi
DEFINES *= REALOPINSIGHT_WEB
DEFINES *= WT_NO_SLOT_MACROS

LIBS += -lwt -lwthttp -lwtdbo -lwtdbosqlite3 \
        -lboost_signals -lboost_program_options-mt -lboost_system-mt \
        -lboost_thread-mt -lboost_regex-mt -lboost_signals-mt \
        -lboost_filesystem-mt -lboost_date_time-mt

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

dflag {
TARGET.path=$$(INSTALL_PREFIX)/bin
MAN.path =$$(INSTALL_PREFIX)/share/man/man1
} else {
TARGET.path=/usr/local/bin
MAN.path = /usr/share/man/man1
}

TARGET.files = $${TARGET}
MAN.files = doc/man/ngrt4n-manager.1.gz doc/man/ngrt4n-oc.1.gz doc/man/ngrt4n-editor.1.gz
INSTALLS += TARGET MAN


