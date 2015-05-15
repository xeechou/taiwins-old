/* There are servral things need ot be done in this file.
 * 1) modesetting
 * 2) setting up tty and the mechanism to release and pick up the context
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>

#include <libudev.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>
#include <EGL/egl.h>

#include <linux/vt.h>
#include <linux/kd.h>

/*This struct is one per connector */;
typedef struct fb_buffer {
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	int scale;

	uint32_t fb[2];
} fb_buffer;
typedef struct disp_info {
	uint32_t conn;
	uint32_t enc;	// we may not need this.
	uint32_t crtc;
	drmModeModeInfo mode;

	//information about the 
	uint32_t location;	/* indicate this is the (x, y)'s display */

	//buffer
	fb_buffer fb;
} disp_info;
typedef struct disp_array {
	int n_disps;
	/* This struct has the geomitry information about which connector is
	   located. */
	int n_vert;	/* how many displays we have vertically    */
	int n_hori;	/* how many displays we have horizentally  */
	struct disp_info *disps;
} disp_array;

//static int modeset_default(int fd);
static int setup_connectors(int fd, drmModeRes *res, disp_array *arr);
static int modeset_set_crtc(int fd, drmModeRes *res,
			    drmModeConnector *conn,
			    disp_info *info);
static int modeset_set_fb(int fd, drmModeRes *res, disp_info *info);


/* crtc and encoder are bond together, so we have to set them together */
static int modeset_set_crtc(int fd, drmModeRes *res,
			    drmModeConnector *conn,
			    disp_info *info)
{
	int i;
	if (!conn) {
		fprintf(stderr, "Cannot retrieve DRM connector %u:%u (%d): %m\n",
			i, res->connectors[i], errno);
		goto err;
	}
	memcpy(&info->mode, &conn->modes[0], sizeof(info->mode));

	/* retrieve a usable encoders */
	drmModeEncoder *enc;
	uint32_t possible_crtcs;

	/* check the current encoder */
	if (conn->encoder_id)
		enc = drmModeGetEncoder(fd, conn->encoder_id);
	if (enc) {
		if (enc->crtc_id)
			info->crtc = enc->crtc_id;
	} else {
		int i;
		for (i = 1; i <= conn->count_encoders; i++) {
			enc = drmModeGetEncoder(fd, conn->encoders[i]);
			if (enc)
				break;
		}
		if (!enc)
			goto err;
		if (enc->crtc_id)
			info->crtc = enc->crtc_id;
		else {
			possible_crtcs = enc->possible_crtcs;
			for(i = 0; i < res->count_crtcs; i++) {
				if (!(possible_crtcs & (1 << i)))
					continue;
				info->crtc = res->crtcs[i];
				//TODO:check crtc is not ocuppied by other connectors
				break;
			}
		}
	}
	drmModeFreeEncoder(enc);
	return 0;
err:
	return errno;
}


static int modeset_set_fb(int fd, drmModeRes *res, disp_info *info)
{
	struct gbm_device *gbm;
	const char *ver, *extensions;
	EGLint major, minor;
	EGLDisplay dpy;		/* void * */

	gbm = gbm_create_device(fd);
	dpy = eglGetDisplay(gbm);
	eglInitialize(dpy, &major, &minor);
        ver = eglQueryString(dpy, EGL_VERSION);
	extensions = eglQueryString(dpy, EGL_EXTENSIONS);
	if (!strstr(extensions, "EGL_KHR_surfaceless_opengl")) {
		fprintf(stderr, "no surfaceless support, cannot initialize\n");
		return -1;;
	}
	const drmModeModeInfo *mode = &info->mode;
	//gbm_bo *bo = gbm_bo_create(gbm,)
}

static int setup_connectors(int fd, drmModeRes *res, disp_array *disp_arr)
{
	uint32_t n_disps;
	int i, ret;
	drmModeConnector *conn;
	disp_info *disps = malloc(sizeof(disp_info) * res->count_connectors);
	//disp_arr->disps = disps;
	disp_arr->n_disps = 0;

	for (i = 0; i < res->count_connectors; i++) {
		conn = drmModeGetConnector(fd, res->connectors[i]);
		if (!conn) {
			fprintf(stderr, "Cannot retrieve DRM connector %u:%u (%d): %m\n",
				i, res->connectors[i], errno);
			continue;
		}
		if (conn->connection != DRM_MODE_CONNECTED) {
			drmModeFreeConnector(conn);
			continue;
		}
		ret = modeset_set_crtc(fd, res, conn, &disps[disp_arr->n_disps]);
		disp_arr->n_disps += 1;
		drmModeFreeConnector(conn);
	}
	if (!disp_arr->n_disps)
		return -1;
	//This will work, we shrink the size
	disp_arr->disps = realloc(disps, sizeof(disp_arr->n_disps *
							  sizeof(disp_info)));
	return 0;

}

int main()
{
	int ret, i;
	int fd = open("/dev/dri/card0", O_RDWR);
	disp_array displays;
	drmModeRes *res;

	res = drmModeGetResources(fd);
	if(!res) {
		fprintf(stderr, "Failed to get DRM resources (%d)!\n", errno);
		return -errno;
	}

	ret = setup_connectors(fd, res, &displays);
	if (ret) {
		fprintf(stderr, "Failed in first call\n");
		return ret;
	}
	fprintf(stdout, "%d\n", displays.n_disps);

	drmModeFreeResources(res);

	return 0;
err:
	return errno;
}
