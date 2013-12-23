QT	+= core gui xml svg webkit network script
CONFIG += no_keywords
TEMPLATE = app
VERSION_=3.0.0
VERSION = "-$${VERSION_}"

config-dist {
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

config-gui-base {
HEADERS	+= include/client/Auth.hpp \
    include/client/GraphView.hpp \
    include/client/PieChart.hpp \
    include/client/PieChartItem.hpp \
    include/client/ServiceEditor.hpp \
    include/client/StatsLegend.hpp \
    include/client/SvConfigCreator.hpp \
    include/client/GuiDashboard.hpp \
    include/client/MainWindow.hpp \
    include/client/SvNavigatorTree.hpp \
    include/client/WebKit.hpp \
    include/client/Settings.hpp \
    include/client/Chart.hpp \
    include/client/MsgConsole.hpp

SOURCES	+= src/client/Auth.cpp \
    src/client/GraphView.cpp \
    src/client/PieChart.cpp \
    src/client/PieChartItem.cpp \
    src/client/ServiceEditor.cpp \
    src/client/SvConfigCreator.cpp \
    src/client/GuiDashboard.cpp \
    src/client/MainWindow.cpp \
    src/client/SvNavigatorTree.cpp \
    src/client/WebKit.cpp \
    src/client/Chart.cpp \
    src/client/MsgConsole.cpp
}

config-web {
TARGET = ngrt4n-web
CONFIG += no_keywords
DEFINES *= REALOPINSIGHT_WEB

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
    wt/src/AuthManager.hpp

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
    wt/src/AuthManager.cpp
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
DEFINES *= "RELEASE_YEAR='\"2014\"'"
DEFINES *= "BUG_REPORT_EMAIL='\"bugs@ngrt4n.com\"'"

DEFINES *=BOOST_TT_HAS_OPERATOR_HPP_INCLUDED

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
QMAKE_CXXFLAGS += -std=c++0x  -Wno-unused-variable  -Wno-unused-parameter -Werror -Wno-unused-local-typedefs
INCLUDEPATH = include \
              librealopinsight/src \
              include/client \
              include/core \
              wt/src \
              wt/dbo \
              /opt/install/wt-3.3.0/include

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

LIBS += -L./librealopinsight/release -lrealopinsight \
    -L/opt/install/wt-3.3.0/lib -lwthttp -lwt -lwtdbo -lwtdbosqlite3 \
    -L/opt/install/lib -lboost_signals -lboost_program_options-mt -lboost_system-mt \
    -lboost_thread-mt -lboost_regex-mt -lboost_signals-mt -lboost_filesystem-mt -lboost_date_time-mt


