### Makefile --- 

## Author: wkretzsch@gmail.com
## Version: $Id: Makefile,v 0.0 2015/04/18 08:56:55 winni Exp $
## Keywords: 
## X-URL: 

CXXFLAGS += -std=c++11 -I.
DEPS = bcf.hpp hts.hpp
LIBS = -lhts

SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
HEADERS=$(wildcard: *.hpp)

all: libhtspp.a
clean:
	rm -f *.a *.o
debug: CXXFLAGS += -g -O0
debug: all

%.o: %.cpp %.hpp
libhtspp.a: $(OBJECTS)
	$(AR) -rv $@ $^

.PHONY: all clean debug test


### testing

TDIR:= test
test: CXXFLAGS += -g -O0
test: $(TDIR)/test_simple
	./$<


$(TDIR)/test_simple: $(TDIR)/test_simple.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

