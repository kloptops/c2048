
# Yay simple :D
CC=gcc

CFLAGS=-c -O3 -Wall -lm
LDFLAGS=-lm

SOURCES=c2048_ai.c  c2048.c  main.c  stdafx.c  tinyrand.c
OBJECTS=$(SOURCES:.c=.o)

EXECUTABLE=c2048

all: $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)
