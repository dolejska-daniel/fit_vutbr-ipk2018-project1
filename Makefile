##
#	Variables
##
CC=gcc
CFLAGS=-Wall -Wextra -pedantic
DFLAGS=-DDEBUG_PRINT_ENABLED -DDEBUG_LOG_ENABLED -DDEBUG_ERR_ENABLED
OUTC=ipk-client
OUTS=ipk-server

##
#	Translate all
##
all:
	make client
	make server

# debug
debug: DEBUG_client.o DEBUG_server.o
	$(CC) $(CFLAGS) client.o -o $(OUTC)
	$(CC) $(CFLAGS) server.o -o $(OUTS)

##
#	Translate client
##
client: client.o
	$(CC) $(CFLAGS) client.o -o $(OUTC)

client.o: client.c
	$(CC) $(CFLAGS) client.c -c

# debug
debug-client: DEBUG_client.o
	$(CC) $(CFLAGS) client.o -o $(OUTC)

# debug
DEBUG_client.o: client.c
	$(CC) $(CFLAGS) $(DFLAGS) client.c -c

##
#	Translate server
##
server: server.o
	$(CC) $(CFLAGS) server.o -o $(OUTS)

server.o: server.c
	$(CC) $(CFLAGS) server.c -c

# debug
debug-server: DEBUG_server.o
	$(CC) $(CFLAGS) server.o -o $(OUTS)

# debug
DEBUG_server.o: server.c
	$(CC) $(CFLAGS) $(DFLAGS) server.c -c

##
#	Clear all
##
clean:
	rm -f *.o $(OUT) $(OUTS) $(OUTC)
