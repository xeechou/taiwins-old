CC=gcc
CFLAGS=-g
DEPFLAGS = `pkg-config --cflags --libs libdrm egl gl gbm`
LOCALLIB = utils.o

all:
	@${CC} ${CLFAGS} -o modeset modeset.c ${DEPFLAGS}
	@${CC} ${CLFAGS} -o vt_set vt_set.c ${LOCALLIB} ${DEPFLAGS}

${LOCALLIB} : ${LOCALLIB:.o=.c}
	${CC} -c $<
clean:
	rm launch vt_set
