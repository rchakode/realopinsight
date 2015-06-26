# ngrt4n.pro
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
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
REALOPINSIGHT_CORE_VERSION=3.2.2
VERSION = "-$${REALOPINSIGHT_CORE_VERSION}"

win32 {
  DEFINES *= WIN32
  DEFINES *= WIN32_LEAN_AND_MEAN
  LIBS += -lws2_32
}

minimal {
  DEFINES *= REALOPINSIGHT_DISABLE_ZMQ
  DEFINES *= REALOPINSIGHT_DISABLE_BROWSER
} else {
  QT +=  webkit
  HEADERS += core/src/ZmqSocket.hpp \
             client/src/WebKit.hpp
  SOURCES += core/src/ZmqSocket.cpp\
             client/src/WebKit.cpp
  win32 {
    INCLUDEPATH += $$PWD/../../../ZeroMQ-2.2.0/include
    LIBS += -L$$PWD/../../../ZeroMQ-2.2.0/bin -llibzmq-v100-mt
  } else {
    LIBS += -lzmq
  }
}

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
    examples/*.ngrt4n.xml \
    doc/man/*.gz \
    i18n/ngrt4n_*.qm

TRANSLATIONS += i18n/ngrt4n_fr.ts
CODECFORSRC = UTF-8
CODECFORTR  = UTF-8

HEADERS += \
    core/src/Base.hpp \
    core/src/Parser.hpp \
    core/src/Preferences.hpp \
    core/src/ZbxHelper.hpp \
    core/src/ZnsHelper.hpp \
    core/src/Settings.hpp \
    core/src/LsHelper.hpp \
    core/src/DashboardBase.hpp \
    core/src/utilsCore.hpp \
    core/src/ChartBase.hpp \
    core/src/JsonHelper.hpp \
    core/src/RawSocket.hpp \
    core/src/ThresholdHelper.hpp \
    core/src/StatusAggregator.hpp \
    core/src/PandoraHelper.hpp \
    core/src/DescriptionFileFactoryUtils.hpp


SOURCES += \
    core/src/Base.cpp \
    core/src/Parser.cpp \
    core/src/Preferences.cpp \
    core/src/ZbxHelper.cpp \
    core/src/ZnsHelper.cpp \
    core/src/Settings.cpp \
    core/src/LsHelper.cpp \
    core/src/DashboardBase.cpp \
    core/src/utilsCore.cpp \
    core/src/ChartBase.cpp \
    core/src/JsonHelper.cpp \
    core/src/RawSocket.cpp \
    core/src/ThresholdHelper.cpp \
    core/src/StatusAggregator.cpp \
    core/src/PandoraHelper.cpp \
    core/src/DescriptionFileFactoryUtils.cpp

QT += svg gui
PACKAGE_VERSION="-$${REALOPINSIGHT_CORE_VERSION}"
HEADERS	+= client/src/Auth.hpp \
    client/src/StatsLegend.hpp \
    client/src/GraphView.hpp \
    client/src/PieChart.hpp \
    client/src/ServiceEditor.hpp \
    client/src/SvConfigCreator.hpp \
    client/src/GuiDashboard.hpp \
    client/src/MainWindow.hpp \
    client/src/SvNavigatorTree.hpp \
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
    client/src/GuiPreferences.cpp \
    client/src/MsgConsole.cpp \
    client/src/GuiDialogForms.cpp

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

DEFINES *= "REALOPINSIGHT_BUILD_DATE=\"1419004446\""
DEFINES *= QT_USE_QSTRINGBUILDER
DEFINES *= "REALOPINSIGHT_BUILTIN_USER_PREFIX='\"ngrt4n\"'"
DEFINES *= "REALOPINSIGHT_APPLICATION_NAME='\"RealOpInsight\"'"
DEFINES *= "REALOPINSIGHT_CORE_VERSION='\"$${REALOPINSIGHT_CORE_VERSION}\"'"
DEFINES *= "REALOPINSIGHT_PACKAGE_VERSION='\"$${REALOPINSIGHT_CORE_VERSION}\"'"
DEFINES *= "REALOPINSIGHT_PACKAGE_URL='\"http://RealOpInsight.com\"'"
DEFINES *= "REALOPINSIGHT_RELEASE_NAME='\"ibericus\"'"
DEFINES *= "REALOPINSIGHT_RELEASE_YEAR='\"2015\"'"
DEFINES *= "REALOPINSIGHT_BUG_REPORT_EMAIL='\"bugs@realopinsight.com\"'"
DEFINES *= "REALOPINSIGHT_GET_HELP_URL='\"http://docs.realopinsight.com/\"'"
DEFINES *= "REALOPINSIGHT_EDITOR_HELP_URL='\"/latest/01_workstation/editor-manual.html\"'"
DEFINES *= "REALOPINSIGHT_DATA_POINT_CONTEXT_URL='\"data-point\"'"


unittests {
SOURCES += core/src/unittests.cpp
QT += testlib
}
