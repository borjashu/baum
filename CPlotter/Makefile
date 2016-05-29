CC = gcc

OBJS = test_CPlotter.o
LIBCPLT = libcplt.a
LIBOBJS = ${LIBCPLT}(CPlotter.o) \
          ${LIBCPLT}(CPLT_intern.o) \
          ${LIBCPLT}(CPLT_EPS.o) \
          ${LIBCPLT}(CPLT_PNG.o) \
          ${LIBCPLT}(CPLT_SVG.o)
LIBS = -lcplt -lm -lgd

CFLAGS = -g -Wall
#CFLAGS = -pg -Wall
#CFLAGS = -g0 -O3

test_CPlotter: ${OBJS} ${LIBOBJS}
	${CC} -o $@ ${OBJS} -L. ${LIBS}

all: ${LIBCPLT}

${OBJS}: CPlotter.h

${LIBOBJS}: CPlotter.h CPLT_intern.h

${LIBCPLT}: ${LIBOBJS}

run:
	./test_CPlotter

clean:
	/bin/rm -f core *.o;

