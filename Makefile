# C-compiler
CC=gcc

CFLAGS=-c -Wall

# Search for .c files in "src" directory; .h files in "include" directory
# The pattern matching character '%' matches filename without the extension
vpath %.c src helpers
vpath %.h include

all: proxy.out showip.out tcpclient.out

install: proxy.out showip.out tcpclient.out
	chmod +x proxy.out
	chmod +x showip.out
	chmod +x tcpclient.out
	mv proxy.out /usr/local/bin/proxy
	mv showip.out /usr/local/bin/showip
	mv tcpclient.out /usr/local/bin/tcpclient

proxy.out: proxy_parse.o proxy.o
	$(CC) -o $@ proxy_parse.o proxy.o

proxy_parse.o:	proxy_parse.c
	$(CC) $(CFLAGS) $<

proxy.o: proxy.c
	$(CC) $(CFLAGS) $<

showip.out: showip.o
	$(CC) -o $@ $<

showip.o: showip.c
	$(CC) $(CFLAGS) $<

tcpclient.out: tcpclient.o
	$(CC) -o $@ $<

tcpclient.o: tcpclient.c
	$(CC) $(CFLAGS) $<

clean:
	rm *.o *.out 2>/dev/null
