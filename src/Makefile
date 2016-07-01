CC = gcc
#CFLAGS=-DNDEBUG -O3
CFLAGS =-Wall -g3 -DDEBUG
LDFLAGS = -lm

SOURCES = main.c board.c data.c bitops.c movegen.c hash.c debug.c io.c perft.c search.c utils.c hashtable.c eval.c
DEPS = globals.h board.h data.h bitops.h movegen.h hash.h debug.h io.h perft.h search.h	utils.h hashtable.h eval.h
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = chess

all: $(SOURCES) $(EXECUTABLE)

nodebug:

%.o: %.c $(DEPS)
		$(CC) $< $(CFLAGS) -c -o $@


$(EXECUTABLE): $(OBJECTS)
		$(CC) $(LDFLAGS) $(OBJECTS) -o $@

#.c.o:
#		$(CC) $< $(CFLAGS) -c -o $@


clean:
	rm -f *.o $(EXECUTABLE)
