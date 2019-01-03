# ngrt4n-web.pro
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2016 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 23-10-2016                                                 #
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

WT_ROOT = $$(WT_ROOT)
QT	+= core xml network script

CONFIG += no_keywords
TEMPLATE = app
RESOURCES += ngrt4n.qrc

PACKAGE_VERSION=2019.r1.0

DEFINES *= "REALOPINSIGHT_RELEASE_NAME='\"Galacia\"'"
DEFINES *= "REALOPINSIGHT_RELEASE_YEAR='\"2019\"'"
DEFINES *= "REALOPINSIGHT_BUILD_DATE=\"`date +%s`\""
DEFINES *= "REALOPINSIGHT_APPLICATION_NAME='\"RealOpInsight\"'"
DEFINES *= "REALOPINSIGHT_PACKAGE_VERSION='\"$${PACKAGE_VERSION}\"'"
DEFINES *= "REALOPINSIGHT_PACKAGE_URL='\"http://realopinsight.com\"'"
DEFINES *= "REALOPINSIGHT_BUG_REPORT_EMAIL='\"support@realopinsight.com\"'"
DEFINES *= "REALOPINSIGHT_GET_HELP_URL='\"http://docs.realopinsight.com/\"'"
DEFINES *= WT_NO_SLOT_MACROS
DEFINES *= BOOST_TT_HAS_OPERATOR_HPP_INCLUDED
DEFINES *= QT_USE_QSTRINGBUILDER

VERSION = "-$${PACKAGE_VERSION}"

OBJECTS_DIR = generated/obj
MOC_DIR = generated/moc
RCC_DIR = generated/rcc
QMAKE_CXXFLAGS += -std=c++14 -Werror -Wno-literal-suffix -Wno-unused-variable -Wno-unused-parameter -Wno-unused-local-typedefs


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
CODECFORTR  = UTF-8

INCLUDEPATH += $(WT_ROOT)/include \
                dbo/src \
                core/src/\
                web/src

HEADERS += \
    core/src/Base.hpp \
    core/src/Parser.hpp \
    core/src/ZbxHelper.hpp \
    core/src/ZnsHelper.hpp \
    core/src/PandoraHelper.hpp \
    core/src/LsHelper.hpp \
    core/src/DashboardBase.hpp \
    core/src/utilsCore.hpp \
    core/src/ChartBase.hpp \
    core/src/JsonHelper.hpp \
    core/src/RawSocket.hpp \
    core/src/ThresholdHelper.hpp \
    core/src/StatusAggregator.hpp \
    core/src/OpManagerHelper.hpp \
    core/src/BaseSettings.hpp \
    core/src/SettingFactory.hpp \
    web/src/utils/wtwithqt/DispatchThread.h \
    web/src/utils/wtwithqt/WQApplication \
    web/src/utils/smtpclient/qxtglobal.h \
    web/src/utils/smtpclient/qxtsmtp.h \
    web/src/utils/smtpclient/qxtsmtp_p.h \
    web/src/utils/smtpclient/qxtmail_p.h \
    web/src/utils/smtpclient/qxthmac.h \
    web/src/utils/smtpclient/qxtmailmessage.h \
    web/src/utils/smtpclient/qxtmailattachment.h \
    web/src/utils/smtpclient/MailSender.hpp \
    web/src/utils/Logger.hpp \
    dbo/src/DbSession.hpp \
    dbo/src/DbObjects.hpp \
    dbo/src/ViewAclManagement.hpp \
    dbo/src/UserManagement.hpp \
    dbo/src/LdapUserManager.hpp \
    dbo/src/NotificationTableView.hpp \
    web/src/WebDashboard.hpp \
    web/src/WebMap.hpp \
    web/src/WebTree.hpp \
    web/src/WebPieChart.hpp \
    web/src/WebMainUI.hpp \
    web/src/WebUtils.hpp \
    web/src/AuthManager.hpp \
    web/src/Validators.hpp \
    web/src/LdapHelper.hpp\
    web/src/AuthModelProxy.hpp \
    web/src/QosCollector.hpp \
    web/src/Applications.hpp \
    web/src/Notificator.hpp \
    web/src/WebCsvReportResource.hpp \
    web/src/WebBiDateFilter.hpp \
    web/src/WebBiDashlet.hpp \
    web/src/WebBiRawChart.hpp \
    web/src/WebBiSlaDataAggregator.hpp \
    web/src/WebMsgDialog.hpp \
    web/src/WebEventConsole.hpp \
    web/src/WebNotificationSettings.hpp \
    web/src/WebDatabaseSettings.hpp \
    web/src/WebDataSourceSettings.hpp \
    web/src/WebBaseSettings.hpp \
    web/src/WebAuthSettings.hpp \ \
    web/src/WebEditor.hpp \
    web/src/WebInputSelector.hpp \
    core/src/K8sHelper.hpp

SOURCES +=  core/src/Base.cpp \
    core/src/Parser.cpp \
    core/src/ZbxHelper.cpp \
    core/src/ZnsHelper.cpp \
    core/src/PandoraHelper.cpp \
    core/src/LsHelper.cpp \
    core/src/DashboardBase.cpp \
    core/src/utilsCore.cpp \
    core/src/ChartBase.cpp \
    core/src/JsonHelper.cpp \
    core/src/RawSocket.cpp \
    core/src/ThresholdHelper.cpp \
    core/src/StatusAggregator.cpp \
    core/src/OpManagerHelper.cpp  \
    core/src/BaseSettings.cpp \
    core/src/SettingFactory.cpp \
    dbo/src/LdapUserManager.cpp \
    dbo/src/NotificationTableView.cpp \
    dbo/src/DbSession.cpp \
    dbo/src/UserManagement.cpp \
    dbo/src/ViewAclManagement.cpp \
    web/src/utils/wtwithqt/DispatchThread.C \
    web/src/utils/wtwithqt/WQApplication.C \
    web/src/utils/smtpclient/qxthmac.cpp \
    web/src/utils/smtpclient/qxtmailattachment.cpp \
    web/src/utils/smtpclient/qxtmailmessage.cpp \
    web/src/utils/smtpclient/qxtsmtp.cpp \
    web/src/utils/smtpclient/MailSender.cpp \
    web/src/utils/Logger.cpp \
    web/src/QosCollector.cpp \
    web/src/WebDashboard.cpp \
    web/src/WebMap.cpp \
    web/src/WebTree.cpp \
    web/src/WebPieChart.cpp \
    web/src/WebMainUI.cpp \
    web/src/WebUtils.cpp \
    web/src/AuthManager.cpp \
    web/src/LdapHelper.cpp \
    web/src/AuthModelProxy.cpp \
    web/src/Notificator.cpp \
    web/src/WebCsvReportResource.cpp \
    web/src/WebBiDashlet.cpp \
    web/src/WebBiDateFilter.cpp \
    web/src/WebBiSlaDataAggregator.cpp \
    web/src/WebBiRawChart.cpp \
    web/src/WebMsgDialog.cpp \
    web/src/WebEventConsole.cpp \
    web/src/WebNotificationSettings.cpp \
    web/src/WebDatabaseSettings.cpp \
    web/src/WebBaseSettings.cpp \
    web/src/WebAuthSettings.cpp \
    web/src/WebDataSourceSettings.cpp \
    web/src/WebEditor.cpp \
    web/src/WebInputSelector.cpp \
    core/src/K8sHelper.cpp


LIBS += -L"$(WT_ROOT)/lib" \
        -lwt \
        -lwtdbo \
        -lwtdbosqlite3 \
        -lwtdbopostgres \
        -lboost_signals \
        -lboost_program_options \
        -lboost_system \
        -lboost_thread \
        -lboost_regex \
        -lboost_signals \
        -lboost_filesystem \
        -lboost_date_time \
        -lldap

reportd {
  SOURCES += web/src/ngrt4n-reportd.cpp
  TARGET = realopinsight-reportd
}

webd {
  SOURCES += web/src/ngrt4n-web.cpp
  TARGET = realopinsight-server
  LIBS += -lwthttp
}

setupdb {
  SOURCES += web/src/ngrt4n-setupdb.cpp
  TARGET = realopinsight-setupdb
}

unittests-web {
  QT += testlib
  TARGET = unittests-web
  HEADERS += web/src/web_foundation_unittests.hpp
  SOURCES += web/src/web_foundation_unittests.cpp
}

unittests-core {
  QT += testlib
  TARGET = unittests-core
  HEADERS += core/src/TestK8sHelper.hpp
  SOURCES += core/src/TestK8sHelper.cpp
}

TARGET.files = $${TARGET}
INSTALLS += TARGET
