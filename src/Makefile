CFLAGS=-std=c99 -Wall -Werror -pthread
.PHONY: all
all: client server test_board test_client_conn test-main test_player

board: board.c
	gcc $(CFLAGS) board.c -o $@ -lncurses

server: server.c player.o list.o tetris-game.o
	gcc $(CFLAGS) list.o tetris-game.o player.o server.c -o $@

render.o: render.c
	gcc $(CFLAGS) -c render.c

client: client.c render.o client_conn.o
	gcc $(CFLAGS) render.o client_conn.o client.c -o $@ -lncurses

test_board: render.o test_board.c
	gcc $(CFLAGS) render.o test_board.c -o $@ -lncurses

client_conn.o: client_conn.c
	gcc $(CFLAGS) -c client_conn.c

test_client_conn: test_client_conn.c client_conn.o
	gcc $(CFLAGS) test_client_conn.c client_conn.o -o $@

list.o: list.c
	gcc $(CFLAGS) -c list.c

player.o: player.c tetris-game.o
	gcc $(CFLAGS) -c player.c tetris-game.o

test_player: test_player.c player.o list.o tetris-game.o
	gcc $(CFLAGS) test_player.c player.o tetris-game.o list.o -o $@

tetris-game.o: tetris-game.c tetris-game.h
	gcc $(CFLAGS) -c tetris-game.c

test-main: test-main.c tetris-game.o
	gcc $(CFLAGS) tetris-game.o test-main.c -o $@

.PHONY: clean
clean:
	rm -f *.out *.o board client server test_board test_client_conn tetris-main
