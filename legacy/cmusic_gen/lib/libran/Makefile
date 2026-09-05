# @(#)Makefile	1.3  19 Sep 1995

PROG		= libran
#CFLAGS =	-O -cckr -I../../../include
CFLAGS =	-O -I../../../include


#OBJS =		frand.o arcsin.o cauchy.o expn.o gamma.o gauss.o hyper.o lin.o \
#		logist.o beta.o plapla.o onefrand.o corrand.o \
#		randfi.o randfc.o
# temporarily deleted gamma.o

OBJS =		frand.o arcsin.o cauchy.o expn.o gauss.o hyper.o lin.o \
		logist.o beta.o plapla.o onefrand.o corrand.o \
		randfi.o randfc.o

#.c.o:
#		${CC} ${CFLAGS} -c $*.c
#		-ld -x -r $*.o
#		mv a.out $*.o

all: ${PROG}

${PROG}: 	${OBJS}
		ar r ../libcarl.a ${OBJS}

install:

clean:
		rm -f ${PROG}.a ${OBJS}

clobber: clean
