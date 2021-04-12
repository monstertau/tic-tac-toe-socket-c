TARGET   = game
CFLAGS = -c -g -Wall
CC = gcc
LIBS =  -lncurses

all: ${TARGET}

${TARGET}:main.o
	${CC} main.o -o ${TARGET} ${LIBS}

main.o: main.c
	${CC} ${CFLAGS} main.c

clean:
	rm -f *.o