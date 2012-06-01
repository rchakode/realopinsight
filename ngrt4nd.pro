TEMPLATE = app
TARGET = build/usr/sbin/ngrt4nd
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc

QT        += 

INCLUDEPATH = include

HEADERS   += include/core/MonitorBroker.hpp
			 
SOURCES   += src/core/MonitorBroker.cpp \
			src/server/ngrt4nd.cpp
			
LIBS += -lcrypt -lzmq

FORMS	  +=
RESOURCES += ngrt4n.qrc
