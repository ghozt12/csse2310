CC=gcc
CFLAGS=-Wall -pedantic -std=gnu99 -g
TARGETS=2310client 2310serv

all: $(TARGETS)

2310serv: 2310serv.c serrs.o deck.o game.o player.o deck.o
	$(CC) $(CFLAGS) -pthread 2310serv.c -o 2310serv serrs.o deck.o player.o game.o

2310client: 2310client.c cerrs.o playmain.o
	$(CC) $(CFLAGS) 2310client.c -o 2310client cerrs.o playmain.o

cerrs.o: cerrs.c cerrs.h
	$(CC) $(CFLAGS) cerrs.c -c

playmain.o: playmain.c playmain.h cerrs.h
	$(CC) $(CFLAGS) -c playmain.c

deck.o: deck.c deck.h
	$(CC) $(CFLAGS) -c deck.c 

game.o:	game.c cards.h game.h deck.h player.h 2310serv.h
	$(CC) $(CFLAGS) -c game.c

player.o: player.c player.h
	$(CC) $(CFLAGS) -c player.c

serrs.o: serrs.c serrs.h
	$(CC) $(CFLAGS) serrs.c -c
clean:
	rm -f $(TARGETS) *.o
