CFLAGS=-std=c99 -Wall -pthread
.PHONY: all
all: client server test_board test_client_conn solo_main test_player

board: board.c
	gcc $(CFLAGS) board.c -o $@ -lncurses

server: server.c player.o list.o tetris_game.o
	gcc $(CFLAGS) list.o tetris_game.o player.o server.c -o $@

render.o: render.c
	gcc $(CFLAGS) -c render.c -lncurses

client: client.c render.o client_conn.o
	gcc $(CFLAGS) render.o client_conn.o client.c -o $@ -lncurses -lpthread

test_board: render.o test_board.c
	gcc $(CFLAGS) render.o test_board.c -o $@ -lncurses

client_conn.o: client_conn.c
	gcc $(CFLAGS) -c client_conn.c -lncurses -lpthread

test_client_conn: test_client_conn.c client_conn.o render.o
	gcc $(CFLAGS) test_client_conn.c client_conn.o render.o -o $@ -lncurses -lpthread

list.o: list.c
	gcc $(CFLAGS) -c list.c

player.o: player.c
	gcc $(CFLAGS) -c player.c

test_player: test_player.c player.o list.o tetris_game.o
	gcc $(CFLAGS) test_player.c player.o tetris_game.o list.o -o $@

tetris_game.o: tetris_game.c tetris_game.h
	gcc $(CFLAGS) -c tetris_game.c

solo_main: solo_main.c tetris_game.o
	gcc $(CFLAGS) tetris_game.o solo_main.c -o $@

.PHONY: clean
clean:
	rm -f *.out *.o board client server test_board test_client_conn tetris-main solo_main test_player
