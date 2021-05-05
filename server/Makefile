BIN_DIR = bin
TEST_DIR = test
TARGET   = server
PROGRAM = ${BIN_DIR}/${TARGET}
MANAGER_TEST = ${TEST_DIR}/game_manager_test
WINNER_TEST = ${TEST_DIR}/winner
CFLAGS = -c -g -Wall
CC = gcc
LIBS =  -lm -pthread

all: ${PROGRAM}
test: ${MANAGER_TEST}
test-winner: ${WINNER_TEST}
${PROGRAM}:main.o game_logic.o game_manager.o msg_parser.o
	${CC} main.o game_logic.o game_manager.o msg_parser.o -o ${PROGRAM} ${LIBS}

main.o: main.c
	${CC} ${CFLAGS} main.c

game_logic.o: game_logic.c
	${CC} ${CFLAGS} game_logic.c

${MANAGER_TEST}: game_manager_test.o game_manager.o
	${CC} game_manager_test.o game_manager.o -o ${MANAGER_TEST}
${WINNER_TEST}: winner.o game_logic.o
	${CC} winner.o game_logic.o -o ${WINNER_TEST}
game_manager_test.o: game_manager_test.c
	${CC} ${CFLAGS} game_manager_test.c

msg_parser.o: msg_parser.c
	${CC} ${CFLAGS} msg_parser.c
game_manager.o: game_manager.c
	${CC} ${CFLAGS} game_manager.c

clean:
	rm -f *.o *~ ${PROGRAM}