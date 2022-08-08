CXX = g++
CXXFLAGS = -w
LDFLAGS = $(CXXFLAGS) -lpthread

.PHONY: bbserv clean distclean

all: bbserv

TULS_TCP.o: TULS_TCP.h TULS_TCP.cc
	$(CXX) $(CXXFLAGS) -c -o TULS_TCP.o TULS_TCP.cc
TIAS.o: TIAS.h
	$(CXX) $(CXXFLAGS) -c -o TIAS.o TIAS.cc

RSEVH.o: TULS_TCP.h TIAS.h FDSH.h RSEVH.cc
	$(CXX) $(CXXFLAGS) -c -o RSEVH.o RSEVH.cc

FSEV.o: TULS_TCP.h FDSH.h FSEV.cc
	$(CXX) $(CXXFLAGS) -c -o FSEV.o FSEV.cc

MCK.o: TULS_TCP.h FDSH.h MCK.cc
	$(CXX) $(CXXFLAGS) -c -o MCK.o MCK.cc

FILES1.o: FILES1.h FILES1.cc
	$(CXX) $(CXXFLAGS) -c -o $@ FILES1.cc

BULLH.o: BULLH.hpp BULLH.cc
	$(CXX) $(CXXFLAGS) -c -o $@ BULLH.cc

SYNC.o: SYNC.cc SNK.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ SYNC.cc

bbserv: TIAS.o TULS_TCP.o RSEVH.o FSEV.o MCK.o FILES1.o BULLH.o SYNC.o
	$(CXX) $(LDFLAGS) -o $@ TIAS.o TULS_TCP.o RSEVH.o FSEV.o MCK.o FILES1.o BULLH.o SYNC.o

## USER:
USER.o: TULS_TCP.h USER.cc
	$(CXX) $(CXXFLAGS) -c -o USER.o USER.cc

USER: USER.o TULS_TCP.o
	$(CXX) -o USER USER.o TULS_TCP.o

shf: USER.o TULS_TCP.o
	$(CXX) $(LDFLAGS) -o shf USER.o TULS_TCP.o

clean:
	rm -f *~ *.o *.bak core \#*

distclean: clean
	rm -f bbserv USER *.log *.pid bbfile
