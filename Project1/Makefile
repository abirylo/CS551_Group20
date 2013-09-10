#MakeFile

CC=gcc
CFLAGS=-c -Wall -g3
LIBS=
LDFLAGS=
SOURCES=shell.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=shell

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(LIBS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@ 
    
clean:
	rm -rf $(OBJECTS) ${EXECUTABLE}    
