QT	+= core gui xml svg webkit network script
TEMPLATE = app
VERSION_=2.5.0
VERSION = "-$${VERSION_}"
config-dist{
SOURCES	+=
TARGET = RealOpInsight
}
config-manager {
SOURCES	+= src/client/ngrt4n-manager.cpp
TARGET = ngrt4n-manager
}
config-oc {
SOURCES	+= src/client/ngrt4n-oc.cpp
TARGET = ngrt4n-oc
}
config-editor {
SOURCES	+= src/client/ngrt4n-editor.cpp
TARGET = ngrt4n-editor
}
config-web {
HEADERS	+= wt/include/WebDashboard.hpp \
    wt/include/WebMsgConsole.hpp \
    wt/include/WebMap.hpp \
    wt/include/WebTree.hpp \
    wt/include/WebUI.hpp \
    wt/include/WebChart.hpp

SOURCES	+= wt/src/WebDashboard.cpp \
    wt/src/WebMsgConsole.cpp \
    wt/src/ngrt4n-web.cpp \
    wt/src/WebUI.cpp \
    wt/src/WebMap.cpp \
    wt/src/WebTree.cpp \
    wt/src/WebChart.cpp

LIBS += -L/opt/install/wt-3.3.0/lib -lwthttp -lwt -lboost_signals
TARGET = ngrt4n-web
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
RESOURCES += ngrt4n.qrc
DEFINES *= QT_USE_QSTRINGBUILDER
DEFINES *= "BUILTIN_USER_PREFIX='\"ngrt4n\"'"
DEFINES *= "APPLICATION_NAME='\"RealOpInsight\"'"
DEFINES *= "PACKAGE_NAME='\"Ultimate\"'"
DEFINES *= "PACKAGE_VERSION='\"$${VERSION_}\"'"
DEFINES *= "PACKAGE_URL='\"http://RealOpInsight.com\"'"
DEFINES *= "RELEASE_NAME='\"Everywhere\"'"
DEFINES *= "RELEASE_YEAR='\"2013\"'"

DEFINES *=BOOST_TT_HAS_OPERATOR_HPP_INCLUDED

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
QMAKE_CXXFLAGS += -std=c++0x -Werror -Wno-unused-local-typedefs
INCLUDEPATH = include include/client \
              include/core \
              wt/include \
              /opt/install/wt-3.3.0/include \

HEADERS	+= include/core/ns.hpp \
    include/core/MonitorBroker.hpp \
    include/client/Auth.hpp \
    include/client/Base.hpp \
    include/client/GraphView.hpp \
    include/client/Parser.hpp \
    include/client/PieChart.hpp \
    include/client/PieChartItem.hpp \
    include/client/Preferences.hpp \
    include/client/ServiceEditor.hpp \
    include/client/StatsLegend.hpp \
    include/client/SvConfigCreator.hpp \
    include/client/GuiDashboard.hpp \
    include/client/MainWindow.hpp \
    include/client/SvNavigatorTree.hpp \
    include/client/WebKit.hpp \
    include/client/ZbxHelper.hpp \
    include/client/JsHelper.hpp \
    include/client/ZnsHelper.hpp \
    include/client/Settings.hpp \
    include/client/utilsClient.hpp \
    include/client/Chart.hpp \
    include/client/MsgConsole.hpp \
    include/core/ZmqSocket.hpp \
    include/client/LsHelper.hpp \
    include/client/DashboardBase.hpp

SOURCES	+= \
    src/core/MonitorBroker.cpp \
    src/client/Auth.cpp \
    src/client/GraphView.cpp \
    src/client/Parser.cpp \
    src/client/PieChart.cpp \
    src/client/PieChartItem.cpp \
    src/client/Preferences.cpp \
    src/client/ServiceEditor.cpp \
    src/client/StatsLegend.cpp \
    src/client/SvConfigCreator.cpp \
    src/client/GuiDashboard.cpp \
    src/client/MainWindow.cpp \
    src/client/SvNavigatorTree.cpp \
    src/client/WebKit.cpp \
    src/client/ZbxHelper.cpp \
    src/client/JsHelper.cpp \
    src/client/ZnsHelper.cpp \
    src/client/Settings.cpp \
    src/client/utilsClient.cpp \
    src/core/utilsCore.cpp \
    src/client/Chart.cpp \
    src/client/MsgConsole.cpp \
    src/core/ZmqSocket.cpp \
    src/client/LsHelper.cpp \
    src/client/DashboardBase.cpp

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
    src/client/ngrt4n-*.cpp \
    doc/man/*.gz \
    i18n/ngrt4n_*.qm

TRANSLATIONS += i18n/ngrt4n_fr.ts

CODECFORSRC = UTF-8

LIBS += -lzmq

#include(QsLog/QsLog.pri)

