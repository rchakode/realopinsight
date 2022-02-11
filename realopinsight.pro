# realopinsight.pro
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
QT	+= core xml network

CONFIG += no_keywords
TEMPLATE = app
RESOURCES += realopinsight.qrc

PACKAGE_VERSION=22.02.1

DEFINES *= "REALOPINSIGHT_RELEASE_NAME='\"Concret\"'"
DEFINES *= "REALOPINSIGHT_RELEASE_YEAR='\"2022\"'"
DEFINES *= "REALOPINSIGHT_BUILD_DATE=\"`date +%s`\""
DEFINES *= "REALOPINSIGHT_APPLICATION_NAME='\"RealOpInsight\"'"
DEFINES *= "REALOPINSIGHT_PACKAGE_VERSION='\"$${PACKAGE_VERSION}\"'"
DEFINES *= "REALOPINSIGHT_PACKAGE_URL='\"http://realopinsight.com\"'"
DEFINES *= "REALOPINSIGHT_BUG_REPORT_EMAIL='\"support@realopinsight.com\"'"
DEFINES *= "REALOPINSIGHT_GET_HELP_URL='\"http://realopinsight.com/docs/\"'"
DEFINES *= WT_NO_SLOT_MACROS
DEFINES *= BOOST_TT_HAS_OPERATOR_HPP_INCLUDED
DEFINES *= QT_USE_QSTRINGBUILDER

VERSION = "-$${PACKAGE_VERSION}"

OBJECTS_DIR = generated/obj
MOC_DIR = generated/moc
RCC_DIR = generated/rcc
QMAKE_CXXFLAGS += -std=c++17 -Werror -Wno-literal-suffix -Wno-unused-variable -Wno-unused-parameter -Wno-unused-local-typedefs


DISTFILES += README.md \
    install-sh \
    install-sh.obs \
    INSTALL \
    LICENSE \
    ChangeLog \
    AUTHORS \
    images/*.png \
    images/built-in/*.png \
    doc/man/*.gz \
    i18n/realopinsight_*.qm \
    resources/font-awesome/font/fontawesome-webfont.woff \
    resources/fonts/fontawesome-webfont.woff \
    contribs/grafana/realopinsight.js \
    resources/jPlayer/jquery.jplayer.min.js \
    resources/jPlayer/jquery.min.js \
    resources/js/ga.js \
    resources/js/jquery-3.4.1.min.js \
    resources/font-awesome/font/fontawesome-webfont.eot \
    resources/fonts/fontawesome-webfont.eot \
    resources/jPlayer/Jplayer.swf \
    resources/font-awesome/font/FontAwesome.otf \
    resources/fonts/FontAwesome.otf \
    resources/font-awesome/font/fontawesome-webfont.ttf \
    resources/fonts/fontawesome-webfont.ttf \
    i18n/realopinsight_fr.qm \
    contribs/backup/realopinsight-backup \
    contribs/backup/realopinsight-restore \
    resources/themes/default/no-stripes/generate.sh \
    resources/themes/default/stripes/generate.sh \
    resources/themes/polished/no-stripes/generate.sh \
    resources/themes/polished/stripes/generate.sh \
    container-entrypoint.sh \
    container-extract-dist.sh \
    helm/templates/tests/test-connection.yaml \
    helm/templates/deployment.yaml \
    helm/templates/ingress.yaml \
    helm/templates/service.yaml \
    helm/templates/serviceaccount.yaml \
    helm/Chart.yaml \
    helm/values.yaml \
    contribs/etc/wt_config.xml \
    resources/i18n/messages.xml \
    resources/jPlayer/skin/jplayer.blue.monday.seeking.gif \
    resources/themes/bootstrap/nav-minus-rtl.gif \
    resources/themes/bootstrap/nav-minus.gif \
    resources/themes/bootstrap/nav-plus-rtl.gif \
    resources/themes/bootstrap/nav-plus.gif \
    resources/themes/bootstrap/sort-arrow-down.gif \
    resources/themes/bootstrap/sort-arrow-none.gif \
    resources/themes/bootstrap/sort-arrow-up.gif \
    resources/themes/default/no-stripes/no-stripe-10px.gif \
    resources/themes/default/no-stripes/no-stripe-11px.gif \
    resources/themes/default/no-stripes/no-stripe-12px.gif \
    resources/themes/default/no-stripes/no-stripe-13px.gif \
    resources/themes/default/no-stripes/no-stripe-14px.gif \
    resources/themes/default/no-stripes/no-stripe-15px.gif \
    resources/themes/default/no-stripes/no-stripe-16px.gif \
    resources/themes/default/no-stripes/no-stripe-17px.gif \
    resources/themes/default/no-stripes/no-stripe-18px.gif \
    resources/themes/default/no-stripes/no-stripe-19px.gif \
    resources/themes/default/no-stripes/no-stripe-20px.gif \
    resources/themes/default/no-stripes/no-stripe-21px.gif \
    resources/themes/default/no-stripes/no-stripe-22px.gif \
    resources/themes/default/no-stripes/no-stripe-23px.gif \
    resources/themes/default/no-stripes/no-stripe-24px.gif \
    resources/themes/default/no-stripes/no-stripe-25px.gif \
    resources/themes/default/no-stripes/no-stripe-26px.gif \
    resources/themes/default/no-stripes/no-stripe-27px.gif \
    resources/themes/default/no-stripes/no-stripe-28px.gif \
    resources/themes/default/no-stripes/no-stripe-29px.gif \
    resources/themes/default/no-stripes/no-stripe-30px.gif \
    resources/themes/default/no-stripes/no-stripe-31px.gif \
    resources/themes/default/no-stripes/no-stripe-32px.gif \
    resources/themes/default/no-stripes/no-stripe-33px.gif \
    resources/themes/default/no-stripes/no-stripe-34px.gif \
    resources/themes/default/no-stripes/no-stripe-35px.gif \
    resources/themes/default/no-stripes/no-stripe-36px.gif \
    resources/themes/default/no-stripes/no-stripe-37px.gif \
    resources/themes/default/no-stripes/no-stripe-38px.gif \
    resources/themes/default/no-stripes/no-stripe-39px.gif \
    resources/themes/default/no-stripes/no-stripe-40px.gif \
    resources/themes/default/stripes/stripe-10px.gif \
    resources/themes/default/stripes/stripe-11px.gif \
    resources/themes/default/stripes/stripe-12px.gif \
    resources/themes/default/stripes/stripe-13px.gif \
    resources/themes/default/stripes/stripe-14px.gif \
    resources/themes/default/stripes/stripe-15px.gif \
    resources/themes/default/stripes/stripe-16px.gif \
    resources/themes/default/stripes/stripe-17px.gif \
    resources/themes/default/stripes/stripe-18px.gif \
    resources/themes/default/stripes/stripe-19px.gif \
    resources/themes/default/stripes/stripe-20px.gif \
    resources/themes/default/stripes/stripe-21px.gif \
    resources/themes/default/stripes/stripe-22px.gif \
    resources/themes/default/stripes/stripe-23px.gif \
    resources/themes/default/stripes/stripe-24px.gif \
    resources/themes/default/stripes/stripe-25px.gif \
    resources/themes/default/stripes/stripe-26px.gif \
    resources/themes/default/stripes/stripe-27px.gif \
    resources/themes/default/stripes/stripe-28px.gif \
    resources/themes/default/stripes/stripe-29px.gif \
    resources/themes/default/stripes/stripe-30px.gif \
    resources/themes/default/stripes/stripe-31px.gif \
    resources/themes/default/stripes/stripe-32px.gif \
    resources/themes/default/stripes/stripe-33px.gif \
    resources/themes/default/stripes/stripe-34px.gif \
    resources/themes/default/stripes/stripe-35px.gif \
    resources/themes/default/stripes/stripe-36px.gif \
    resources/themes/default/stripes/stripe-37px.gif \
    resources/themes/default/stripes/stripe-38px.gif \
    resources/themes/default/stripes/stripe-39px.gif \
    resources/themes/default/stripes/stripe-40px.gif \
    resources/themes/default/nav-minus-rtl.gif \
    resources/themes/default/nav-minus.gif \
    resources/themes/default/nav-plus-rtl.gif \
    resources/themes/default/nav-plus.gif \
    resources/themes/default/slider-thumb-h-disabled.gif \
    resources/themes/default/slider-thumb-h.gif \
    resources/themes/default/slider-thumb-v-disabled.gif \
    resources/themes/default/slider-thumb-v.gif \
    resources/themes/default/sort-arrow-disabled.gif \
    resources/themes/default/sort-arrow-down.gif \
    resources/themes/default/sort-arrow-none.gif \
    resources/themes/default/sort-arrow-up.gif \
    resources/themes/polished/no-stripes/no-stripe-10px.gif \
    resources/themes/polished/no-stripes/no-stripe-11px.gif \
    resources/themes/polished/no-stripes/no-stripe-12px.gif \
    resources/themes/polished/no-stripes/no-stripe-13px.gif \
    resources/themes/polished/no-stripes/no-stripe-14px.gif \
    resources/themes/polished/no-stripes/no-stripe-15px.gif \
    resources/themes/polished/no-stripes/no-stripe-16px.gif \
    resources/themes/polished/no-stripes/no-stripe-17px.gif \
    resources/themes/polished/no-stripes/no-stripe-18px.gif \
    resources/themes/polished/no-stripes/no-stripe-19px.gif \
    resources/themes/polished/no-stripes/no-stripe-20px.gif \
    resources/themes/polished/no-stripes/no-stripe-21px.gif \
    resources/themes/polished/no-stripes/no-stripe-22px.gif \
    resources/themes/polished/no-stripes/no-stripe-23px.gif \
    resources/themes/polished/no-stripes/no-stripe-24px.gif \
    resources/themes/polished/no-stripes/no-stripe-25px.gif \
    resources/themes/polished/no-stripes/no-stripe-26px.gif \
    resources/themes/polished/no-stripes/no-stripe-27px.gif \
    resources/themes/polished/no-stripes/no-stripe-28px.gif \
    resources/themes/polished/no-stripes/no-stripe-29px.gif \
    resources/themes/polished/no-stripes/no-stripe-30px.gif \
    resources/themes/polished/no-stripes/no-stripe-31px.gif \
    resources/themes/polished/no-stripes/no-stripe-32px.gif \
    resources/themes/polished/no-stripes/no-stripe-33px.gif \
    resources/themes/polished/no-stripes/no-stripe-34px.gif \
    resources/themes/polished/no-stripes/no-stripe-35px.gif \
    resources/themes/polished/no-stripes/no-stripe-36px.gif \
    resources/themes/polished/no-stripes/no-stripe-37px.gif \
    resources/themes/polished/no-stripes/no-stripe-38px.gif \
    resources/themes/polished/no-stripes/no-stripe-39px.gif \
    resources/themes/polished/no-stripes/no-stripe-40px.gif \
    resources/themes/polished/stripes/stripe-10px.gif \
    resources/themes/polished/stripes/stripe-11px.gif \
    resources/themes/polished/stripes/stripe-12px.gif \
    resources/themes/polished/stripes/stripe-13px.gif \
    resources/themes/polished/stripes/stripe-14px.gif \
    resources/themes/polished/stripes/stripe-15px.gif \
    resources/themes/polished/stripes/stripe-16px.gif \
    resources/themes/polished/stripes/stripe-17px.gif \
    resources/themes/polished/stripes/stripe-18px.gif \
    resources/themes/polished/stripes/stripe-19px.gif \
    resources/themes/polished/stripes/stripe-20px.gif \
    resources/themes/polished/stripes/stripe-21px.gif \
    resources/themes/polished/stripes/stripe-22px.gif \
    resources/themes/polished/stripes/stripe-23px.gif \
    resources/themes/polished/stripes/stripe-24px.gif \
    resources/themes/polished/stripes/stripe-25px.gif \
    resources/themes/polished/stripes/stripe-26px.gif \
    resources/themes/polished/stripes/stripe-27px.gif \
    resources/themes/polished/stripes/stripe-28px.gif \
    resources/themes/polished/stripes/stripe-29px.gif \
    resources/themes/polished/stripes/stripe-30px.gif \
    resources/themes/polished/stripes/stripe-31px.gif \
    resources/themes/polished/stripes/stripe-32px.gif \
    resources/themes/polished/stripes/stripe-33px.gif \
    resources/themes/polished/stripes/stripe-34px.gif \
    resources/themes/polished/stripes/stripe-35px.gif \
    resources/themes/polished/stripes/stripe-36px.gif \
    resources/themes/polished/stripes/stripe-37px.gif \
    resources/themes/polished/stripes/stripe-38px.gif \
    resources/themes/polished/stripes/stripe-39px.gif \
    resources/themes/polished/stripes/stripe-40px.gif \
    resources/themes/polished/nav-minus-rtl.gif \
    resources/themes/polished/nav-minus.gif \
    resources/themes/polished/nav-plus-rtl.gif \
    resources/themes/polished/nav-plus.gif \
    resources/themes/polished/sort-arrow-disabled.gif \
    resources/themes/polished/sort-arrow-down.gif \
    resources/themes/polished/sort-arrow-none.gif \
    resources/themes/polished/sort-arrow-up.gif \
    resources/ajax-loading.gif \
    resources/collapse-rtl.gif \
    resources/collapse.gif \
    resources/expand-rtl.gif \
    resources/expand.gif \
    resources/items-not-ok.gif \
    resources/items-ok.gif \
    resources/items.gif \
    resources/line-last-rtl.gif \
    resources/line-last.gif \
    resources/line-middle-rtl.gif \
    resources/line-middle.gif \
    resources/line-trunk.gif \
    resources/minus.gif \
    resources/plus.gif \
    resources/right-arrow.gif \
    resources/tab_b.gif \
    resources/tab_l.gif \
    resources/tab_r.gif \
    resources/tv-line-last-rtl.gif \
    resources/tv-line-last.gif \
    resources/jPlayer/skin/jplayer.blue.monday.jpg \
    resources/jPlayer/skin/jplayer.blue.monday.video.play.png \
    resources/themes/bootstrap/2/date-edit-button.png \
    resources/themes/bootstrap/2/spin-buttons.png \
    resources/themes/bootstrap/2/suggest-dropdown.png \
    resources/themes/bootstrap/3/date-edit-button.png \
    resources/themes/bootstrap/3/spin-buttons.png \
    resources/themes/bootstrap/3/suggest-dropdown.png \
    resources/themes/bootstrap/date-edit-button.png \
    resources/themes/bootstrap/glyphicons-halflings-white.png \
    resources/themes/bootstrap/glyphicons-halflings.png \
    resources/themes/bootstrap/spin-buttons.png \
    resources/themes/bootstrap/splitter-h.png \
    resources/themes/bootstrap/splitter-v.png \
    resources/themes/default/closeicons-dialog.png \
    resources/themes/default/closeicons-mi.png \
    resources/themes/default/closeicons-tab.png \
    resources/themes/default/dropdown.png \
    resources/themes/default/sliderbg-h-disabled.png \
    resources/themes/default/sliderbg-h.png \
    resources/themes/default/sliderbg-v-disabled.png \
    resources/themes/default/sliderbg-v.png \
    resources/themes/default/spin-buttons-dn.png \
    resources/themes/default/spin-buttons-up.png \
    resources/themes/default/spin-buttons.png \
    resources/themes/default/splitter-h.png \
    resources/themes/default/splitter-v.png \
    resources/themes/img/glyphicons-halflings.png \
    resources/themes/polished/closeicons-dialog.png \
    resources/themes/polished/closeicons-mi.png \
    resources/themes/polished/closeicons-tab.png \
    resources/themes/polished/dropdown.png \
    resources/themes/polished/dropshadow.png \
    resources/themes/polished/gradient.png \
    resources/themes/polished/nav-minus-rtl.png \
    resources/themes/polished/nav-minus.png \
    resources/themes/polished/nav-plus-rtl.png \
    resources/themes/polished/nav-plus.png \
    resources/themes/polished/slider-thumb-disabled.png \
    resources/themes/polished/slider-thumb.png \
    resources/themes/polished/sliderbg-h-disabled.png \
    resources/themes/polished/sliderbg-h.png \
    resources/themes/polished/sliderbg-he-disabled.png \
    resources/themes/polished/sliderbg-he.png \
    resources/themes/polished/sliderbg-hw-disabled.png \
    resources/themes/polished/sliderbg-hw.png \
    resources/themes/polished/sliderbg-v-disabled.png \
    resources/themes/polished/sliderbg-v.png \
    resources/themes/polished/sliderbg-ve-disabled.png \
    resources/themes/polished/sliderbg-ve.png \
    resources/themes/polished/sliderbg-vw-disabled.png \
    resources/themes/polished/sliderbg-vw.png \
    resources/themes/polished/splitter-h.png \
    resources/themes/polished/splitter-v.png \
    resources/themes/polished/suggest-dropdown.png \
    resources/themes/polished/title-gradient.png \
    resources/calendar_edit.png \
    resources/loading.png \
    resources/resizable.png \
    resources/font-awesome/font/fontawesome-webfont.svg \
    resources/fonts/fontawesome-webfont.svg \
    resources/themes/bootstrap/2/date-edit-button.svg \
    resources/themes/bootstrap/2/spin-buttons.svg \
    resources/themes/bootstrap/2/suggest-dropdown.svg \
    resources/themes/bootstrap/3/date-edit-button.svg \
    resources/themes/bootstrap/3/spin-buttons.svg \
    resources/themes/bootstrap/3/suggest-dropdown.svg \
    resources/themes/bootstrap/date-edit-button.svg \
    resources/themes/bootstrap/spin-buttons.svg \
    resources/css/kube/custom.css \
    resources/css/kube/font.css \
    resources/css/kube/highlight.css \
    resources/css/kube/kube.css \
    resources/css/kube/kube.demo.css \
    resources/css/kube/kube.legenda.css \
    resources/css/kube/kube.min.css \
    resources/css/kube/master.css \
    resources/css/font-awesome.min.css \
    resources/css/realopinsight.css \
    resources/font-awesome/css/font-awesome-ie7.css \
    resources/font-awesome/css/font-awesome-ie7.min.css \
    resources/font-awesome/css/font-awesome.css \
    resources/font-awesome/css/font-awesome.min.css \
    resources/jPlayer/skin/jplayer.blue.monday.css \
    resources/themes/bootstrap/2/bootstrap-responsive.css \
    resources/themes/bootstrap/2/bootstrap-responsive.min.css \
    resources/themes/bootstrap/2/bootstrap.css \
    resources/themes/bootstrap/2/bootstrap.min.css \
    resources/themes/bootstrap/2/wt.css \
    resources/themes/bootstrap/3/bootstrap-theme.css \
    resources/themes/bootstrap/3/bootstrap-theme.min.css \
    resources/themes/bootstrap/3/bootstrap.css \
    resources/themes/bootstrap/3/bootstrap.min.css \
    resources/themes/bootstrap/3/wt.css \
    resources/themes/bootstrap/bootstrap-responsive.css \
    resources/themes/bootstrap/bootstrap-responsive.min.css \
    resources/themes/bootstrap/bootstrap.css \
    resources/themes/bootstrap/bootstrap.min.css \
    resources/themes/bootstrap/wt.css \
    resources/themes/default/wt.css \
    resources/themes/default/wt_ie.css \
    resources/themes/default/wt_ie6.css \
    resources/themes/polished/wt.css \
    resources/themes/polished/wt_ie.css \
    resources/themes/polished/wt_ie6.css \
    resources/form.css \
    resources/html4_default.css \
    resources/moz-transitions.css \
    resources/transitions.css \
    resources/webkit-transitions.css \
    helm/templates/_helpers.tpl \
    helm/templates/NOTES.txt \
    resources/font-awesome/LICENSE.txt \
    resources/themes/bootstrap/2/wt.less \
    resources/themes/bootstrap/3/bootstrap-theme.css.map \
    resources/themes/bootstrap/3/bootstrap.css.map \
    resources/themes/bootstrap/3/wt.less \
    resources/themes/bootstrap/notes.txt \
    resources/themes/bootstrap/wt.less \
    Dockerfile \
    Dockerfile.builder \
    install-manager \
    i18n/realopinsight_fr.ts \
    contribs/k8s/serviceaccount.yaml \
    contribs/k8s/get_service_account_token.sh

TRANSLATIONS += i18n/realopinsight_fr.ts
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
    core/src/LsHelper.hpp \
    core/src/DashboardBase.hpp \
    core/src/utilsCore.hpp \
    core/src/ChartBase.hpp \
    core/src/RawSocket.hpp \
    core/src/ThresholdHelper.hpp \
    core/src/StatusAggregator.hpp \
    core/src/BaseSettings.hpp \
    core/src/SettingFactory.hpp \
    web/src/utils/wtwithqt/DispatchThread.h \
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
    dbo/src/UserManagement.hpp \
    dbo/src/LdapUserManager.hpp \
    dbo/src/NotificationTableView.hpp \
    web/src/WebDashboard.hpp \
    web/src/WebMap.hpp \
    web/src/WebPieChart.hpp \
    web/src/WebMainUI.hpp \
    web/src/WebUtils.hpp \
    web/src/AuthManager.hpp \
    web/src/Validators.hpp \
    web/src/LdapHelper.hpp\
    web/src/AuthModelProxy.hpp \
    web/src/Notificator.hpp \
    web/src/WebCsvReportResource.hpp \
    web/src/WebMsgDialog.hpp \
    web/src/WebEventConsole.hpp \
    web/src/WebNotificationSettings.hpp \
    web/src/WebDatabaseSettings.hpp \
    web/src/WebDataSourceSettings.hpp \
    web/src/WebBaseSettings.hpp \
    web/src/WebAuthSettings.hpp \ \
    web/src/WebEditor.hpp \
    core/src/K8sHelper.hpp \
    dbo/src/ViewAccessControl.hpp \
    web/src/utils/wtwithqt/WQApplication.h \
    web/src/WebTree.hpp \
    web/src/WebApplication.hpp \
    web/src/WebInputField.hpp \
    web/src/WebInputList.hpp \
    web/src/WebPlatformStatusDateFilter.hpp \
    web/src/WebPlatformStatusRaw.hpp \
    web/src/PlatformStatusCollector.hpp \
    web/src/WebPlatformStatusAnalyticsData.hpp \
    web/src/WebPlatformStatusAnalyticsCharts.hpp

SOURCES +=  core/src/Base.cpp \
    core/src/Parser.cpp \
    core/src/ZbxHelper.cpp \
    core/src/LsHelper.cpp \
    core/src/DashboardBase.cpp \
    core/src/utilsCore.cpp \
    core/src/ChartBase.cpp \
    core/src/RawSocket.cpp \
    core/src/ThresholdHelper.cpp \
    core/src/StatusAggregator.cpp \
    core/src/BaseSettings.cpp \
    core/src/SettingFactory.cpp \
    dbo/src/LdapUserManager.cpp \
    dbo/src/NotificationTableView.cpp \
    dbo/src/DbSession.cpp \
    dbo/src/UserManagement.cpp \
    web/src/utils/wtwithqt/DispatchThread.C \
    web/src/utils/wtwithqt/WQApplication.C \
    web/src/utils/smtpclient/qxthmac.cpp \
    web/src/utils/smtpclient/qxtmailattachment.cpp \
    web/src/utils/smtpclient/qxtmailmessage.cpp \
    web/src/utils/smtpclient/qxtsmtp.cpp \
    web/src/utils/smtpclient/MailSender.cpp \
    web/src/utils/Logger.cpp \
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
    web/src/WebMsgDialog.cpp \
    web/src/WebEventConsole.cpp \
    web/src/WebNotificationSettings.cpp \
    web/src/WebDatabaseSettings.cpp \
    web/src/WebBaseSettings.cpp \
    web/src/WebAuthSettings.cpp \
    web/src/WebDataSourceSettings.cpp \
    web/src/WebEditor.cpp \
    core/src/K8sHelper.cpp \
    dbo/src/ViewAccessControl.cpp \
    web/src/WebApplication.cpp \
    web/src/WebInputField.cpp \
    web/src/WebInputList.cpp \
    web/src/WebPlatformStatusDateFilter.cpp \
    web/src/PlatformStatusCollector.cpp \
    web/src/WebPlatformStatusAnalyticsData.cpp \
    web/src/WebPlatformStatusAnalyticsCharts.cpp \
    web/src/WebPlatformStatusRaw.cpp


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
  SOURCES += web/src/realopinsight-reportd.cpp
  TARGET = realopinsight-reportd
  LIBS += -lprometheus-cpp-core -lprometheus-cpp-push -lprometheus-cpp-pull
}

server {
  SOURCES += web/src/realopinsight-server.cpp
  TARGET = realopinsight-server
  LIBS += -lwthttp
}

setupdb {
  SOURCES += web/src/realopinsight-setupdb.cpp
  TARGET = realopinsight-setupdb
}

unittests-web {
  QT += testlib
  TARGET = unittests-web
  HEADERS += web/src/web_foundation_unittests.hpp \
    web/src/web_foundation_unittests.hpp
  SOURCES += web/src/web_foundation_unittests.cpp
    web/src/web_foundation_unittests.cpp \
}

unittests-core {
  QT += testlib
  TARGET = unittests-core
  HEADERS += core/src/TestK8sHelper.hpp \
    core/src/TestK8sHelper.hpp \
  SOURCES += core/src/TestK8sHelper.cpp \
    unittests/tst_mklshelpertest.cpp \
    unittests/web_foundation_unittest.cpp \
    core/src/TestK8sHelper.cpp \
    core/src/unittests.cpp
}

TARGET.files = $${TARGET}
INSTALLS += TARGET
