CC = gcc
LDFLAGS = -lrt
RM = /bin/rm -f

.SUFFIXES : .c .o

INC =
LIBS =
CFLAGS = -D_REENTRANT  -pthread -g -w #w 워닝지우는거 -Wall 모든워닝다 띄우기

LIBOBJS = ${LIBSRC:.c=.o}

SERVER = server
CLIENT = client

$@.o : $@.c
	$(CC) $(CFLAGS) -c -o $<

all: sv cl

sv: ${LIBOBJS} ${OBJECTS}
	${CC} -o ${SERVER} ${SERVER:=.c} ${LDFLAGS} ${LIBOBJS} ${CFLAGS}

cl: ${LIBOBJS} ${OBJECTS2}
	${CC} -o ${CLIENT} ${CLIENT:=.c} ${LDFLAGS} ${LIBOBJS} ${CFLAGS}

rm :
	${RM} ${SERVER} ${CLIENT} ${LIBOBJS} core tags
