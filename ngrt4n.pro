QT	+= core gui xml svg webkit network script
TEMPLATE = app
VERSION_=2.2.0beta1
VERSION = "-$${VERSION_}"
LIBS += -lzmq
config-dist{
SOURCES	+=
TARGET = RealOpInsight
}
config-manager{
SOURCES	+= src/client/ngrt4n-manager.cpp
TARGET = ngrt4n-manager
}
config-oc{
SOURCES	+= src/client/ngrt4n-oc.cpp
TARGET = ngrt4n-oc
}
config-editor{
SOURCES	+= src/client/ngrt4n-editor.cpp
TARGET = ngrt4n-editor
}
dflag{
TARGET.path=$$(INSTALL_PREFIX)/bin
}else{
TARGET.path=/usr/local/bin
}
TARGET.files = $${TARGET}
MAN.path = /usr/share/man/man1
MAN.files = doc/man/ngrt4n-manager.1.gz doc/man/ngrt4n-oc.1.gz doc/man/ngrt4n-editor.1.gz
INSTALLS += TARGET MAN
RESOURCES += ngrt4n.qrc
DEFINES *= QT_USE_QSTRINGBUILDER
DEFINES *= "BUILTIN_USER_PREFIX='\"ngrt4n\"'"
DEFINES *= "APPLICATION_NAME='\"RealOpInsight\"'"
DEFINES *= "PACKAGE_NAME='\"\"'"
DEFINES *= "PACKAGE_VERSION='\"$${VERSION_}\"'"
DEFINES *= "PACKAGE_URL='\"http://RealOpInsight.com\"'"
DEFINES *= "RELEASE_INFO='\"02/2013\"'"
DEFINES *= "RELEASE_NAME='\"Lucia\"'"
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
QMAKE_CXXFLAGS += -std=c++0x
INCLUDEPATH = include include/client include/core
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
    include/client/SvNavigator.hpp \
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
    include/client/MkLsHelper.hpp

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
    src/client/SvNavigator.cpp \
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
    src/client/MkLsHelper.cpp


TRANSLATIONS += ngrt4n_en.ts

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
    doc/man/*.gz

OTHER_FILES += ngrt4n_la.ts
