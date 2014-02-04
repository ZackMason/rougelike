CFLAGS	= -g

all: main keyboard

main: main.o
	$(CC) -o main main.o -lncurses

keyboard: keyboard.o
	$(CC) -o keyboard keyboard.o -lncurses
