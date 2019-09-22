CFLAGS=-std=c99 -Wall -pthread
.PHONY: all
all: solo-main

tetris-game.o: tetris-game.c tetris-game.h
	gcc $(CFLAGS) -c tetris-game.c

solo-main: solo-main.c tetris-game.o
	gcc $(CFLAGS) tetris-game.o solo-main.c -o $@

.PHONY: clean
clean:
	rm -f *.out *.o solo-main
