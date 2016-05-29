CC = gcc

OBJS = template.o template_funcs.o
INCDIR = CPlotter
LIBS = -lcplt -lm -lgd
#LIBS = -lcplt -lm

CFLAGS = -g -Wall -I${INCDIR}
#CFLAGS = -g0 -O3 -I${INCDIR}

template: ${OBJS} plt_obj
	${CC} -o $@ ${OBJS} -L${INCDIR} ${LIBS}

${OBJS}: template_funcs.h ${INCDIR}/CPlotter.h

plt_obj:
	cd ${INCDIR}; ${MAKE} all "CC=${CC}" "CFLAGS=${CFLAGS}"

clean:
	/bin/rm -f core *.o; cd ${INCDIR}; ${MAKE} clean

