#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <gbm.h>
#include <EGL/egl.h>
//#include <EGL/eglext.h>
//#include <EGL/eglmesaext.h>
#include "renderer.h"

static const EGLint RENDER_ATTRIBUTES[] = {
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_RED_SIZE, 1,
	EGL_GREEN_SIZE, 1,
	EGL_BLUE_SIZE, 1,
	EGL_ALPHA_SIZE, 1,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	EGL_NONE
};

static const EGLint VISUAL_IDS[] = {
	GBM_FORMAT_XRGB8888,
	GBM_FORMAT_ARGB8888
};

//static EGLint PBUFFER_ATTRIBUTES[] = {
//   EGL_WIDTH,		3200,
//   EGL_HEIGHT,		1800,
//   EGL_NONE
//};

int init_gbm(int fd, render_handler *rh)
{
	rh->gbm = gbm_create_device(fd);
	if (!rh->gbm)
		goto err;
	//info->bo = gbm_bo_create(info->gbm, info->width, info->height,
	//			GBM_BO_FORMAT_XRGB8888,
	//			GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
	//if (!info->bo)
	//	goto err;
	return 0;
err:
	return -errno;
}

static int match_visual_id(EGLDisplay display, EGLint visual_id, EGLConfig
		*configs, const int count)
{
	int i;
	for (i = 0; i < count; i++) {
		EGLint gbm_format;
		if (!eglGetConfigAttrib(display, configs[i], EGL_NATIVE_VISUAL_ID,
					&gbm_format))
			continue;
		if (gbm_format == visual_id)
			return i;
	}
	return -1;

}

static int choose_config(render_handler *rh, const EGLint *visual_id, 
		const int n_ids, EGLint *return_vid)
{
	int i;
	EGLint count = 0, matched = 0;
	EGLBoolean success;
	EGLConfig *configs;
	int config_id = -1;

	success = eglGetConfigs(rh->dpy, NULL, 0, &count);
	if (!success || count < 1)
		goto err_get_config;
	if (!(configs = calloc(count, sizeof(EGLConfig))))
		goto err_get_config;
	success = eglChooseConfig(rh->dpy, RENDER_ATTRIBUTES, configs, count,
			     &matched);
	if (!success || !matched)
		goto err_choose_config;

	if (!visual_id)
		goto err_choose_config;
	//match desired visual id
	for (i = 0; config_id == -1 &&i < n_ids; i++)
		config_id = match_visual_id(rh->dpy, visual_id[i],
				configs, matched);
	if (config_id == -1)
		goto out;

	rh->cfg = configs[config_id];
	*return_vid = visual_id[i];
out:
	free(configs);
	if (config_id == -1)
		return -1;
	return 0;

err_choose_config:
	free(configs);
err_get_config:
	return eglGetError();
}

static int check_platform_extensions(void)
{
	const char *extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	if (!strstr(extensions, "EGL_MESA_platform_gbm"))
		return 0;
	return 1;
}
int init_egl(render_handler *rh, uint32_t width, uint32_t height)
{
	int major, minor;
	int platform = 0;
	EGLint chosed_visual_id = 0;

	platform = check_platform_extensions();
	//TODO
#ifdef EGL_MESA_platform_gbm
	rh->dpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_MESA, rh->gbm,
					     NULL);
#else
	rh->dpy = eglGetDisplay(rh->gbm);
#endif
	if (rh->dpy == EGL_NO_DISPLAY)
		return -EINVAL;
	eglInitialize(rh->dpy, &major, &minor);
	eglBindAPI(EGL_OPENGL_API);
	if (choose_config(rh, VISUAL_IDS, sizeof(VISUAL_IDS) / sizeof(EGLint),
			&chosed_visual_id))
		return -EINVAL;

	rh->native_window = gbm_surface_create(rh->gbm,
						 width, height,
						 chosed_visual_id,
						 GBM_BO_USE_SCANOUT |
						 GBM_BO_USE_RENDERING);
	if (!rh->native_window)
		return -1;
#ifdef EGL_MESA_platform_gbm
	rh->sfs = eglCreatePlatformWindowSurfaceEXT(rh->dpy,
						    rh->cfg,
			       (EGLNativeWindowType)rh->native_window,
						    NULL);
#else
	rh->sfs = eglCreateWindowSurface(rh->dpy,
					 rh->cfg,
		    (EGLNativeWindowType)rh->native_window,
					 NULL);
#endif
	if (rh->sfs == EGL_NO_SURFACE) {
		fprintf(stderr, "fail to create surface\n");
		return eglGetError();
	}
	return 0;

}

void destroy_render_handler(render_handler *rh)
{
	if (rh->gbm)
		gbm_device_destroy(rh->gbm);
}
