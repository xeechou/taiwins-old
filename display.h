#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <libudev.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>
#include <EGL/egl.h>
#include "renderer.h"

typedef struct render_handler {
	struct gbm_device *gbm;	// you can alloc 16 gbm at most
	struct gbm_bo *bo;
	EGLConfig  cfg;
	EGLDisplay dpy;
	EGLContext ctx;

	struct gbm_surface *native_window; //use this to create this
	EGLSurface sfs;
} render_handler;

typedef struct disp_info {
	uint32_t conn;
	uint32_t enc;	// we may not need this.
	uint32_t crtc;
	drmModeModeInfo mode;
	uint32_t location;	// indicate this is the (x, y)'s display
				//(1, 1) means the first display

	//fb physical info
	uint32_t width, height;
	uint32_t x, y;		// rendering start at the framebuffer
	uint32_t stride;
	int scale;

	//frambuffer info
	struct render_handler rh;
	uint32_t fb[2];		// for page flip

} disp_info;

/* disp_array represent the physical screens */
typedef struct disp_array {
	int n_disps;
	/* This struct has the geomitry information about which connector is
	   located. */
	uint32_t range;	/* how many displays we have on x and y */
	struct disp_info *disps;
} disp_array;

#endif
