TARGET   = game
CFLAGS = -c -g -Wall
CC = gcc
LIBS =  -lncurses

all: ${TARGET}

${TARGET}:cli.o msg_parser.o
	${CC} cli.o msg_parser.o -o ${TARGET} ${LIBS}

cli.o: cli.c
	${CC} ${CFLAGS} cli.c
msg_parser.o: msg_parser.c
	${CC} ${CFLAGS} msg_parser.c
clean:
	rm -f *.o