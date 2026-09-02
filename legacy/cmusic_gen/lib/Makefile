# @(#)Makefile	1.3  19 Sep 1995
#
# makefile for libraries: libcarl.a 

LIBDIR =	../../lib
LIBS = libcarl.a

all: $(LIBS)

libcarl.a:
	rm -f libcarl.a
	(cd libfrm; make all)
	(cd libdgl; make all)
	(cd libprocom; make all)
	(cd libran; make all)
	(ranlib libcarl.a)



install: $(LIBS)
	install -f ${LIBDIR} $(LIBS)

clean:
	rm -f $(LIBS)
	(cd libfrm; make clean)
	(cd libdgl; make clean)
	(cd libprocom; make clean)
	(cd libran; make clean)

clobber: clean
