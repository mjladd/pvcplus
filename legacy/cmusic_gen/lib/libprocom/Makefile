# @(#)Makefile	1.3  19 Sep 1995

INCLUDE =	../../include
LIBDIR =	../../../lib
#CFLAGS =	-g -cckr -I$(INCLUDE)
CFLAGS =	-g -I$(INCLUDE)

PROG =		libprocom

OBJS =	fgetfloat.o	fputfloat.o getheader.o putheader.o \
		putprop.o  getprop.o  finitbuf.o fflushfloat.o putfc.o \
		cpioheader.o  mkprop.o  globals.o

# REMOVED OBJECTS
#  

all: ${PROG}

${PROG}: 	${OBJS}
		ar r ../libcarl.a ${OBJS}

install:

clean:
		rm -f ${OBJS}

clobber: clean

${OBJS}:	$(INCLUDE)/carl/procom.h
