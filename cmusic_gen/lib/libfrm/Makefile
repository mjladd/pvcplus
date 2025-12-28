# @(#)Makefile	1.3  19 Sep 1995

LIBDIR =	../../../lib
CFLAGS =	-O
#CFLAGS =	-O -cckr

PROG =		libfrm
OBJS =		expr.o polish.o trans.o

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
