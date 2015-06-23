CC=gcc
CFLAGS= -Wall -g
DEPFLAGS = `pkg-config --cflags --libs libdrm egl gl gbm`
LOCALLIB = utils.o
DEMO = demo

all:
	@${CC} ${CFLAGS} -o modeset modeset.c renderer.c ${DEPFLAGS}
	@${CC} ${CFLAGS} -o vt_set vt_set.c ${LOCALLIB} ${DEPFLAGS}

${LOCALLIB} : ${LOCALLIB:.o=.c}
	${CC} -c $<

#%.d: %.c
#	${CC} ${CFLAGS} -c $<
#	${DEPEND} $< | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@
#
#-include ${CSRC:.c=.d}

clean:
	rm launch vt_set
