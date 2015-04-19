### Makefile --- 

## Author: wkretzsch@gmail.com
## Version: $Id: Makefile,v 0.0 2015/04/18 08:56:55 winni Exp $
## Keywords: 
## X-URL: 

CXXFLAGS += -std=c++11

all: libhtspp.a
clean:
	rm -f *.a *.o
debug: CXXFLAGS += -g -O0
debug: all

%.o: %.cpp %.hpp
libhtspp.a: bcf.o
	ar -rv $@ $^


