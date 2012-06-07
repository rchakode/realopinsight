TEMPLATE = app
TARGET = build/usr/sbin/ngrt4nd
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc

QMAKE_CXXFLAGS += -std=c++0x
QT        +=

INCLUDEPATH = include include/core

HEADERS   += include/core/MonitorBroker.hpp
			 
SOURCES   += src/core/MonitorBroker.cpp \
		src/core/utils.cpp \
			src/server/ngrt4nd.cpp
			
LIBS += -lcrypt -lzmq

FORMS	  +=
RESOURCES += ngrt4n.qrc
