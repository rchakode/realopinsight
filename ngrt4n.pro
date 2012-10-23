QT	+= core gui xml svg webkit network script
TEMPLATE = app
VERSION_=2.1.0beta1
VERSION = "-$${VERSION_}"
LIBS += -lzmq
#QMAKE_TARGET=RealOpInsight
config-dist{
SOURCES	+= src/client/ngrt4n.cpp
TARGET = RealOpInsight
}
config-default{
SOURCES	+= src/client/ngrt4n.cpp
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
MAN.files = doc/man/row-manager.1.gz
INSTALLS += TARGET MAN
RESOURCES += ngrt4n.qrc
DEFINES *= QT_USE_QSTRINGBUILDER
DEFINES *= "BUILTIN_USER_PREFIX='\"ngrt4n\"'"
DEFINES *= "APPLICATION_NAME='\"RealOpInsight\"'"
DEFINES *= "PACKAGE_NAME='\"\"'"
DEFINES *= "PACKAGE_VERSION='\"$${VERSION_}\"'"
DEFINES *= "PACKAGE_URL='\"http://RealOpInsight.com\"'"
DEFINES *= "RELEASE_YEAR='\"2012\"'"
DEFINES *= "RELEASE_NAME='\"M-Sylvie\"'"
OBJECTS_DIR = build/obj
MOC_DIR 	= build/moc
RCC_DIR 	= build/rcc 
QMAKE_CXXFLAGS += -std=c++0x
INCLUDEPATH = include include/client include/core 
HEADERS	+= include/core/ns.hpp \
			 include/core/MonitorBroker.hpp \
                         include/core/ZmqHelper.hpp \
			 include/client/Auth.hpp \
			 include/client/Base.hpp \
			 include/client/GraphView.hpp \
			 include/client/MsgPanel.hpp \
			 include/client/Parser.hpp \
			 include/client/PieChart.hpp \
			 include/client/PieChartItem.hpp \
			 include/client/Preferences.hpp \
			 include/client/ServiceEditor.hpp \
			 include/client/Stats.hpp \
			 include/client/StatsLegend.hpp \
			 include/client/SvConfigCreator.hpp \
			 include/client/SvNavigator.hpp \
			 include/client/SvNavigatorTree.hpp \
			 include/client/WebKit.hpp \  			 
    include/client/Utils.hpp \
    include/client/ZbxHelper.hpp \
    include/client/JsHelper.hpp
SOURCES	+= src/core/utils.cpp \
			src/core/MonitorBroker.cpp \
                        src/core/ZmqHelper.cpp \
			src/client/Auth.cpp \
			src/client/Base.cpp \
			src/client/GraphView.cpp \
			src/client/MsgPanel.cpp \ 
            src/client/Parser.cpp \
            src/client/PieChart.cpp \
            src/client/PieChartItem.cpp \
            src/client/Preferences.cpp \
            src/client/ServiceEditor.cpp \
            src/client/Stats.cpp \
            src/client/StatsLegend.cpp \
            src/client/SvConfigCreator.cpp \
            src/client/SvNavigator.cpp \
            src/client/SvNavigatorTree.cpp \
            src/client/WebKit.cpp \  
    src/client/Utils.cpp \
    src/client/ZbxHelper.cpp \
    src/client/JsHelper.cpp


TRANSLATIONS = ngrt4n_en.ts

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
			 src/client/ngrt4n-oc.cpp \
                         src/client/ngrt4n-editor.cpp \
                         doc/man/*.gz

OTHER_FILES = \
    ngrt4n_la.ts
