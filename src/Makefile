CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lm

SOURCES = main.c board.c data.c bitops.c movegen.c uci.c hash.c debug.c io.c perft.c
DEPS = globals.h board.h data.h bitops.h movegen.h hash.h debug.h perft.h
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = chess

all: $(SOURCES) $(EXECUTABLE)

%.o: %.c $(DEPS)
		$(CC) $< $(CFLAGS) -c -o $@


$(EXECUTABLE): $(OBJECTS)
		$(CC) $(LDFLAGS) $(OBJECTS) -o $@

#.c.o:
#		$(CC) $< $(CFLAGS) -c -o $@


clean:
	rm -f *.o $(EXECUTABLE)
