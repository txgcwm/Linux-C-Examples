#
#
#


CC = gcc
PROG_NAME = fileclient fileserver 
INCS = common/common.h
SRCS = fileclient.c	\
	   fileserver.c

OBJS = ${SRCS:.c=.o}

LIBS=

all:${PROG_NAME}

fileclient:fileclient.o common/common.o
	${CC} -o $@ $^

fileserver:fileserver.o common/common.o
	${CC} -o $@ $^

${OBJS}:${INCS}

.c.o:
	${CC} -c $< -I common

common/common.o:common/common.c common/common.h
	cd common && ${CC} -c common.c

clean:
	rm -f *.o ${PROG_NAME} common/*.o

rebuild:clean all 
