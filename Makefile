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
COMMUTE = commute
PRINT = print

$@.o : $@.c
	$(CC) $(CFLAGS) -c -o $<

all: sv cl

sv: ${LIBOBJS} ${SERVER:=.o} ${COMMUTE:=.o} ${PRINT:=.o}
	${CC} -o ${SERVER} ${SERVER:=.o} ${COMMUTE:=.o} ${PRINT:=.o} ${LDFLAGS} ${LIBOBJS} ${CFLAGS}

cl: ${LIBOBJS} ${CLINET:=.o} ${COMMUTE:=.o} ${PRINT:=.o}
	${CC} -o ${CLIENT} ${CLIENT:=.o} ${COMMUTE:=.o} ${PRINT:=.o} ${LDFLAGS} ${LIBOBJS} ${CFLAGS}

rm :
	${RM} *.o ${SERVER} ${CLIENT} ${LIBOBJS} core tags
