.PHONY: all
all: client server test_board test_client_conn test-main

board: board.c
	gcc -lncurses -std=c11 -Wall -Wextra board.c -o $@

server: server.c
	gcc -std=c11 -Wall -Wextra server.c -o $@

render.o: render.c
	gcc -std=c99 -Wall -Wextra -c render.c

client: client.c render.o client_conn.o
	gcc -std=c99 -Wall -Wextra -lncurses render.o client_conn.o client.c -o $@

test_board: render.o test_board.c
	gcc -std=c99 -Wall -Werror -lncurses render.o test_board.c -o $@

client_conn.o: client_conn.c
	gcc -std=c99 -Wall -Werror -c client_conn.c

test_client_conn: test_client_conn.c client_conn.o
	gcc -std=c99 -Wall -Werror test_client_conn.c client_conn.o -o $@

list.o: list.c
	gcc -std=c99 -Wall -Werror -c list.c

player.o: player.c
	gcc -std=c99 -Wall -Werror -c player.c

test_player: test_player.c player.o list.o
	gcc -std=c99 -Wall -Werror test_player.c player.o list.o

tetris-game.o: tetris-game.c tetris-game.h
	gcc -std=c99 -Wall -c tetris-game.c

test-main: test-main.c tetris-game.o
	gcc -std=c99 -Wall tetris-game.o test-main.c

.PHONY: clean
clean:
	rm -f *.out *.o board client server test_board test_client_conn tetris-main
