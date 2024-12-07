CC=gcc
CFLAGS= -g -Wall -Werror

all: proxy309

proxy309: proxy.c
	$(CC) $(CFLAGS) -o proxy_parse.o -c proxy_parse.c
	$(CC) $(CFLAGS) -o proxy.o -c proxy.c
	$(CC) $(CFLAGS) -o proxy309 proxy_parse.o proxy.o

clean:
	rm -f proxy309 *.o
