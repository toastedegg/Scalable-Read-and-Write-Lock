IDIR=../../include
EDIR=../include
LDIR=../.. 
CC=g++
CFLAGS=-g
CXX=g++
CXXFLAGS=-std=c++11 -g -lpthread


ODIR=.

LIBS=

_DEPS = mcsqueue.h rwlock.h node.h SNZI.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = mcsqueue.o rwlock.o SNZIReaderCounter.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o:%.c $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(LIBS)

rwlock: $(OBJ) 
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)



.PHONY: clean

clean:
	rm -f $(ODIR)/*.o rwlock
