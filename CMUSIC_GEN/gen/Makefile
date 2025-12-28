DESTDIR =	../../bin
CFLAGS =	-O -I../include -I/opt/local/include
#CFLAGS =	-O -cckr -I../include
#LIBS =		../lib/libcarl.a
LIBS =		../lib/libcarl.a  -lm


PROGS = cspline gen1 gen2 gen3 gen4 gen5 gen6 cannon

lib:		
	(cd ../lib ; make )

all:	$(PROGS)

cspline: cspline.o ${OBJS}
	${CC} cspline.o ${OBJS} ${LIBS} -o cspline

gen1: gen1.o ${OBJS}
	${CC} gen1.o ${OBJS} ${LIBS} -o gen1

gen2: gen2.o ${OBJS}
	${CC} gen2.o ${OBJS} ${LIBS} -o gen2

gen3: gen3.o ${OBJS}
	${CC} gen3.o ${OBJS} ${LIBS} -o gen3

gen4: gen4.o ${OBJS}
	${CC} gen4.o ${OBJS} ${LIBS} -o gen4

gen5: gen5.o ${OBJS}
	${CC} gen5.o ${OBJS} ${LIBS} -o gen5

gen6: gen6.o ${OBJS}
	${CC} gen6.o ${OBJS} ${LIBS} -o gen6

#step: step.o ${OBJS}
#	${CC} step.o ${OBJS} ${LIBS} -o step

cannon: cannon.o ${OBJS}
	${CC} cannon.o ${OBJS} ${LIBS} -o cannon



install: ${PROGS}
	strip ${PROGS}
#	install -f ${DESTDIR} ${PROGS}
	install ${PROGS} ${DESTDIR} 


clean:
	rm -f ${PROGS} ${PROGS:=.o} nodfun.o

clobber: clean
