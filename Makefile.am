AM_CXXFLAGS=-std=c++0x -Wall -I$(top_srcdir)/include  -I$(top_srcdir)/include/core -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""
AM_LDFLAGS=$(SUBLIBS) -lcrypt -lzmq -lpthread

bin_PROGRAMS = ngrt4nd
ngrt4nd_SOURCES =  src/core/MonitorBroker.cpp src/core/utils.cpp src/server/ngrt4nd.cpp
