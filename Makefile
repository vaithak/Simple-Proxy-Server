# C-compiler
CC=gcc

CFLAGS=-c -Wall

# Search for .c files in "src" directory; .h files in "include" directory
# The pattern matching character '%' matches filename without the extension
vpath %.c src
vpath %.h include

all: showip

install: showip
	chmod +x showip
	mv showip /usr/local/bin

showip: showip.o
	$(CC) -o $@ $<

showip.o: showip.c
	$(CC) $(CFLAGS) $<

clean:
	rm *.o showip 
	
