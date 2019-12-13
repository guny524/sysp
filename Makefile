CC = gcc
LDFLAGS = -lrt
RM = /bin/rm -f

.SUFFIXES : .c .o

INC =
LIBS =
CFLAGS = -D_REENTRANT  -pthread -g -w #w 워닝지우는거 -Wall 모든워닝다 띄우기

LIBOBJS = ${LIBSRC:.c=.o}

CHAT_SERVER = chat_server
CHAT_CLIENT = chat_client
GAME_SERVER = game_server
GAME_CLIENT = game_client

$@.o : $@.c
	$(CC) $(CFLAGS) -c -o $<

all: chat game

chat: chat_server chat_client

game: game_server game_client

chat_server: ${LIBOBJS} ${CHAT_SERVER:=.o}
	${CC} -o ${CHAT_SERVER} ${CHAT_SERVER:=.o} ${LDFLAGS} ${LIBOBJS} ${CFLAGS}

chat_client: ${LIBOBJS} ${CHAT_CLIENT:=.o}
	${CC} -o ${CHAT_CLIENT} ${CHAT_CLIENT:=.o} ${LDFLAGS} ${LIBOBJS} ${CFLAGS}

game_server: ${LIBOBJS} ${GAME_SERVER:=.o}
	${CC} -o ${GAME_SERVER} ${GAME_SERVER:=.o} ${LDFLAGS} ${LIBOBJS} ${CFLAGS}

game_client: ${LIBOBJS} ${GAME_CLIENT:=.o}
	${CC} -o ${GAME_CLIENT} ${GAME_CLIENT:=.o} ${LDFLAGS} ${LIBOBJS} ${CFLAGS}

rm :
	${RM} *.o ${CHAT_SERVER} ${CHAT_CLIENT} ${GAME_SERVER} ${GAME_CLIENT} ${LIBOBJS} core tags
