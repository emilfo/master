CC = gcc
CFLAGS = -Wall
LDFLAGS =

SOURCES = main.c board.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = chess

all: $(SOURCES) $(EXECUTABLE)

%.o: %.c
		$(CC) $< $(CFLAGS) -c -o $@


$(EXECUTABLE): $(OBJECTS)
		$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
		$(CC) $< $(CFLAGS) -c -o $@


clean:
	rm -f *.o $(EXECUTABLE)
