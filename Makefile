CFLAGS	= -g

all: main

main: main.o
	$(CC) -o main main.o -lncurses
