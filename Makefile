CC=gcc
CFLAGS=-g
DEPFLAGS = `pkg-config --cflags --libs libdrm egl gl gbm`
LOCALLIB = utils.o
${LOCALLIB} : ${LOCALLIB:.o=.c}
	${CC} -c $<

all:
	@${CC} ${CLFAGS} -o modeset modeset.c ${DEPFLAGS}
	@${CC} ${CLFAGS} -o vt_set vt_set.c ${LOCALLIB} ${DEPFLAGS}
clean:
	rm launch vt_set
