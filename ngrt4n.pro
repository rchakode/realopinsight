TEMPLATE = app
TARGET = build/usr/bin/ngrt4n
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc

QT        += core gui xml svg webkit 

HEADERS   += include/Auth.hpp \
			 include/Base.hpp \
			 include/GraphView.hpp \
			 include/MsgPanel.hpp \
			 include/Parser.hpp \
			 include/PieChart.hpp \
			 include/PieChartItem.hpp \
			 include/PreferencesDialog.hpp \
			 include/ServiceEditor.hpp \
			 include/Stats.hpp \
			 include/StatsLegend.hpp \
			 include/SvConfigCreator.hpp \
			 include/SvNavigator.hpp \
			 include/SvNavigatorTree.hpp \
			 include/WebKit.hpp 
SOURCES   += src/ngrt4n.cpp \
			src/Auth.cpp \
			src/Base.cpp \
			src/GraphView.cpp \
			src/MsgPanel.cpp \ 
            src/Parser.cpp \
            src/PieChart.cpp \
            src/PieChartItem.cpp \
            src/PreferencesDialog.cpp \
            src/ServiceEditor.cpp \
            src/Stats.cpp \
            src/StatsLegend.cpp \
            src/SvConfigCreator.cpp \
            src/SvNavigator.cpp \
            src/SvNavigatorTree.cpp \
            src/WebKit.cpp 
FORMS	  +=
RESOURCES += ngrt4n.qrc
