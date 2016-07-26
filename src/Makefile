CC = gcc
LDFLAGS =-lm -lpthread
################################################################################
# Options:
# debug=y
# 		- Makes the program with DDEBUG flag, enabling asserts
# profile=y
# 		- To profile the program using gprof, only really useful with 1 thread
# 		  cannot be used together with debug-option
################################################################################

ifeq ($(debug), y)
	CFLAGS=-Wall -Wextra -Wno-long-long -DDEBUG -g3
else ifeq ($(profile), y)
	CFLAGS=-Wall -Wextra -Wno-long-long -DNDEBUG -pg
	LDFLAGS += -pg
else
	CFLAGS=-Wall -Wextra -Wno-long-long -DNDEBUG -o3
endif


SOURCES = main.c board.c data.c bitops.c movegen.c hash.c debug.c io.c perft.c search.c utils.c hashtable.c eval.c uci.c threads.c
DEPS = globals.h board.h data.h bitops.h movegen.h hash.h debug.h io.h perft.h search.h	utils.h hashtable.h eval.h uci.h threads.h defs.h
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = KholinCE


all: $(SOURCES) $(EXECUTABLE)


%.o: %.c $(DEPS)
		$(CC) $< $(CFLAGS) -c -o $@


$(EXECUTABLE): $(OBJECTS)
		$(CC) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	rm -f *.o $(EXECUTABLE)
