#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <gbm.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
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

/*** Functions ***/
static PFNEGLGETPLATFORMDISPLAYEXTPROC get_platform_display = NULL;
static PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC create_platform_surface = NULL;

//static EGLint PBUFFER_ATTRIBUTES[] = {
//   EGL_WIDTH,		3200,
//   EGL_HEIGHT,		1800,
//   EGL_NONE
//};

int init_gbm(int fd, render_handler *rh)
{
	rh->gbm = gbm_create_device(fd);
	if (!rh->gbm)
		return -errno;
	return 0;
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
	/* TODO:now we only check gbm, adding more later */
	const char *extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	if (!strstr(extensions, "EGL_EXT_platform_base"))
		return 0;
	if (!strstr(extensions, "EGL_MESA_platform_gbm"))
		return 0;
	return 1;
}

/* init_egl create display and surface with gbm support */
int init_egl(render_handler *rh, uint32_t width, uint32_t height)
{
	int major, minor;
	int platform = 0;
	EGLint chosed_visual_id = 0;

	platform = check_platform_extensions();
	//STEP 0: get display
	//TODO: get platform extensions
	if (platform) {
		get_platform_display =
			(void*) eglGetProcAddress("eglGetPlatformDisplayEXT");
		rh->dpy = get_platform_display(EGL_PLATFORM_GBM_MESA, rh->gbm,
						NULL);
	}
	else
		rh->dpy = eglGetDisplay(rh->gbm);

	if (rh->dpy == EGL_NO_DISPLAY)
		return -EINVAL;
	eglInitialize(rh->dpy, &major, &minor);
	eglBindAPI(EGL_OPENGL_API);
	if (choose_config(rh, VISUAL_IDS, sizeof(VISUAL_IDS) / sizeof(EGLint),
			&chosed_visual_id))
		return -EINVAL;


	//STEP 1: get surface
	rh->native_window = gbm_surface_create(rh->gbm,
						 width, height,
						 chosed_visual_id,
						 GBM_BO_USE_SCANOUT |
						 GBM_BO_USE_RENDERING);
	if (!rh->native_window)
		return -1;
	if (platform) {
		create_platform_surface = (void *)
			eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT");
	rh->sfs = create_platform_surface(rh->dpy,
					  rh->cfg,
					  (EGLNativeWindowType)rh->native_window,
					  NULL);
	} else
		rh->sfs = eglCreateWindowSurface(rh->dpy,
						 rh->cfg,
			    (EGLNativeWindowType)rh->native_window,
						 NULL);
	if (rh->sfs == EGL_NO_SURFACE) {
		fprintf(stderr, "fail to create surface\n");
		return eglGetError();
	}

	//STEP 2: create context
	static const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	rh->ctx = eglCreateContext(rh->dpy, rh->cfg, EGL_NO_CONTEXT,
				   context_attribs);
	if (rh->ctx == NULL)
		return eglGetError();

	//STEP 3: make current
	return eglMakeCurrent(rh->dpy, rh->sfs, rh->sfs, rh->ctx);
}


void sample_render(render_handler *rh)
{
	//some gl/gles code, I have to learn something before write this
}


void use_buffer(int fd, disp_info *info)
{
	render_handler *rh = &info->rh;
	struct gbm_bo *bo;
	uint32_t handle, stride;
	int fb, ret;
	drmModeCrtcPtr *saved_crtc;

	eglSwapBuffers(rh->dpy, rh->sfs);

	//prepare for adding fb
	bo = gbm_surface_lock_front_buffer(rh->native_window);
	handle = gbm_bo_get_handle(bo).u32;
	stride = gbm_bo_get_pitch(bo);


	ret = drmModeAddFB(fd, info->width, info->height, 24, 32,
			   stride, handle, &fb);
	if (ret)
		; //log this
	ret = drmModeSetCrtc(fd, info->crtc, fb, 0, 0, &info->conn, &info->mode);
	if (ret)
		printf("failed to set mode\n");
	while (1) {
		struct gbm_bo *next_bo;
		eglSwapBuffer(rh->dpy, rh->sfs);
		if (gbm_surface_has_free_buffers(rh->native_window))
			next_bo =
				gbm_surface_lock_front_buffer(rh->native_window);
		ret = drmModeSetCrtc(fd, info->crtc, fb, 0, 0, &info->conn, &info->mode);
		if (ret)
			printf("failed to set mode\n");
	}
}
void destroy_render_handler(render_handler *rh)
{
	//TODO: egl has destroy functions as well, you will really need to
	//handle that
	if (rh->native_window)
		gbm_surface_destroy(rh->native_window);
	if (rh->gbm)
		gbm_device_destroy(rh->gbm);
}
