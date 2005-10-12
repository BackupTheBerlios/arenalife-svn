CFLAGS=$(shell pkg-config glib-2.0 --cflags) -g -Wall $(COPTS)
LDFLAGS=$(shell pkg-config glib-2.0 --libs)
LDLIBS=-lpthread -lncurses
CC=ccache gcc
CPP=ccache g++
TARGETS=arena injecter
OBJS= netw.o celula.o slicer.o main.o cpu.o arena.o instructor.o memmanager.o estadistica.o ui.o render.o mutation.o genbank.o

TEST_LIBS = tests/MemSuite.h

DEPS= memmanager.c estadistica.c mutation.c 

all : $(TARGETS)

arena:  $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)

injecter: injecter.o instructor.o
	$(CC) -o $@ $^ $(LDFLAGS) 

tests: runner
	./runner

runner: runner.o
	$(CPP) -o $@ runner.o $(DEPS) $(CFLAGS) $(LDFLAGS)

runner.o: runner.cpp
	$(CPP) -c $^ $(CFLAGS) $(LDFLAGS)

runner.cpp: $(TEST_LIBS)
	make clean
	rm -f runner.cpp
	cxxtestgen.pl --error-printer -o $@ $^
			
clean: 
	rm -f $(TARGETS) *.o core* *~ *.bak* gb/* runner runner.cpp
