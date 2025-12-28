h65279
s 00002/00000/00028
d D 1.3 95/09/19 16:03:51 loomis 3 2
c 
e
s 00003/00002/00025
d D 1.2 95/09/19 12:54:21 loomis 2 1
c 
e
s 00027/00000/00000
d D 1.1 95/08/28 18:40:15 loomis 1 0
c date and time created 95/08/28 18:40:15 by loomis
e
u
U
f e 0
t
T
I 1
# %W%  %G%

PROG		= libran
CFLAGS =	-O -cckr -I../../../include

#OBJS =		frand.o arcsin.o cauchy.o expn.o gamma.o gauss.o hyper.o lin.o \
#		logist.o beta.o plapla.o frand.o onefrand.o corrand.o \
#		randfi.o randfc.o
# temporarily deleted gamma.o

OBJS =		frand.o arcsin.o cauchy.o expn.o gauss.o hyper.o lin.o \
		logist.o beta.o plapla.o frand.o onefrand.o corrand.o \
		randfi.o randfc.o

#.c.o:
#		${CC} ${CFLAGS} -c $*.c
#		-ld -x -r $*.o
#		mv a.out $*.o

I 2
all: ${PROG}

E 2
${PROG}: 	${OBJS}
		ar r ../libcarl.a ${OBJS}

D 2
install:	${PROG}
		mv ${PROG} ../../../lib/${PROG}.a
E 2
I 2
install:
E 2

clean:
		rm -f ${PROG}.a ${OBJS}
I 3

clobber: clean
E 3
E 1
