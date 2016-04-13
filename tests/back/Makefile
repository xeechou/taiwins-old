CC=gcc
CFLAGS= -Wall -g
DEPFLAGS = `pkg-config --cflags --libs wayland-client wayland-server  wlc cairo`
LOCALLIB = utils.o
COMPLIB = background_extern.o example.o code.o
DEMO = demo

all: ${LOCALLIB} code.o
	@${CC} ${CFLAGS} -o cai cairo.c code.o ${LOCALLIB} ${DEPFLAGS}

compositor: ${COMPLIB}
	@${CC} ${CFLAGS} -o comp ${COMPLIB} ${DEPFLAGS}

${LOCALLIB} : ${LOCALLIB:.o=.c}
	${CC} -c $<

#%.d: %.c
#	${CC} ${CFLAGS} -c $<
#	${DEPEND} $< | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@
#
#-include ${CSRC:.c=.d}

clean:
	rm launch vt_set
