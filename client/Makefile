CLI_TARGET   = cli
UI_TARGET = ui
DRAW_GAME_TARGET = game_design
CFLAGS = -c -g -Wall
CC = gcc
LIBS =  -lncurses

all: ${CLI_TARGET} ${UI_TARGET}


${CLI_TARGET}:cli.o msg_parser.o
	${CC} cli.o msg_parser.o -o ${CLI_TARGET} ${LIBS}

${UI_TARGET}: UI.o msg_parser.o view.o
	${CC} UI.o msg_parser.o view.o -o ${UI_TARGET} ${LIBS}

${DRAW_GAME_TARGET}: view.o
	${CC}  view.o -o ${DRAW_GAME_TARGET} ${LIBS}

cli.o: cli.c
	${CC} ${CFLAGS} cli.c

UI.o: UI.c
	${CC} ${CFLAGS} UI.c
view.o: view.c
	${CC} ${CFLAGS} view.c
msg_parser.o: msg_parser.c
	${CC} ${CFLAGS} msg_parser.c

clean:
	rm -f *.o