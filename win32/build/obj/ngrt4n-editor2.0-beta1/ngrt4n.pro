QT	+= core gui xml svg webkit
TEMPLATE = app
VERSION = 2.0-beta1
LIBS += -lzmq
FORMS	  +=
config-default{
SOURCES	+= src/client/ngrt4n.cpp
TARGET = ngrt4n
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
MAN.files = doc/man/ngrt4n.1.gz
INSTALLS += TARGET MAN
RESOURCES += ngrt4n.qrc
DEFINES += "APPLICATION_NAME='\"NGRT4N\"'"
DEFINES += "PACKAGE_NAME='\"ngrt4n\"'"
DEFINES += "PACKAGE_VERSION='\"$$VERSION\"'"
DEFINES += "PACKAGE_URL='\"http://ngrt4n.com\"'"
DEFINES += "RELEASE_YEAR='\"2012\"'"
OBJECTS_DIR = build/obj
MOC_DIR 	= build/moc
RCC_DIR 	= build/rcc 
QMAKE_CXXFLAGS += -std=c++0x
INCLUDEPATH = include include/client include/core 
HEADERS	+= include/core/ns.hpp \
			 include/core/MonitorBroker.hpp \
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
			 include/client/WebKit.hpp 			 
SOURCES	+= src/core/utils.cpp \
			src/core/MonitorBroker.cpp \
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
            src/client/WebKit.cpp 
DISTFILES += README \
			 INSTALL \
			 COPYING \
			 NEWS \
			 ChangeLog \
			 AUTHORS \
			 install-sh \
			 images/*.png \
			 images/built-in/*.png \
			 examples/{*.xml,*.dat} \
			 src/client/ngrt4n-oc.cpp \
			 src/client/ngrt4n-editor.cpp \
			 src/server/ngrt4nd.cpp \
			 doc/man/*.gz \
			 bkr-1.0-beta1.tar.gz