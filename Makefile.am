AM_CXXFLAGS=-std=c++0x -Wall -I$(top_srcdir)/include  -I$(top_srcdir)  -I$(top_srcdir)/include/core -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""
AM_LDFLAGS=$(SUBLIBS) -lcrypt -lzmq

man1_MANS = doc/man/ngrt4nd.1.gz
sbin_PROGRAMS = ngrt4nd
ngrt4nd_SOURCES =  src/core/MonitorBroker.cpp \
		   src/core/utils.cpp \
		   src/server/ngrt4nd.cpp \
		   include/core/MonitorBroker.hpp \
		   include/core/ns.hpp \
		   include/core/ZmqHelper.hpp \
		   src/core/ZmqHelper.cpp

dist_man_MANS =  doc/man/ngrt4nd.1.gz
