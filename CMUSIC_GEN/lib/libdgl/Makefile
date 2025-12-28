# @(#)Makefile	1.3  19 Sep 1995

CFLAGS =	-O -I../../include
#CFLAGS =	-O -cckr -I../../include
PROG =		libdgl
OBJS =		crack.o floatsav.o 

#.c.o:
#		${CC} ${CFLAGS} -c $*.c
#		-ld -x -r $*.o
#		mv a.out $*.o

all: ${PROG}

${PROG}: 	${OBJS}
		ar r ../libcarl.a ${OBJS}

install:

clean:
		rm -f ${OBJS}

clobber: clean
