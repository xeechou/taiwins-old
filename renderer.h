#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "display.h"

extern int init_egl(render_handler *, uint32_t, uint32_t);
extern int init_gbm(int fd, render_handler *rh);
extern void destroy_render_handler(render_handler *rh);
#endif

