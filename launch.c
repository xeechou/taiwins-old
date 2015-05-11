/* There are servral things need ot be done in this file.
 * 1) modesetting
 * 2) setting up tty and the mechanism to release and pick up the context
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>

#include <libudev.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <linux/vt.h>
#include <linux/kd.h>

/*This struct is one per connector */;
typedef struct fb_buffer {
	uint32_t width;
	uint32_t height;
	int scale;

	uint32_t fb;
} fb_buffer;
typedef struct disp_info {
	uint32_t conn;
	uint32_t enc;	// we may not need this.
	uint32_t crtc;

	drmModeModeInfo mode;

	//information about the 
	uint32_t location;	/* indicate this is the (x, y)'s display */
} disp_info;
typedef struct disp_array {
	/* This struct has the geomitry information about which connector is
	   located. */
	int n_vert;	/* how many displays we have vertically    */
	int n_hori;	/* how many displays we have horizentally  */
	struct disp_info *disps;
} disp_array;

static int modeset_default(int fd);
static int modeset_setup(int fd, drmModeRes *res, disp_info *info);	//not extenable

static int modeset_default(int fd)
{
	/* For default version, we only open one display, can this code be
	 * generalized later? */
	int ret;
	disp_info info;

	drmModeRes *res = drmModeGetResources(fd);
	if(!res) {
		fprintf(stderr, "Failed to get DRM resources (%d)!\n", errno);
		return -errno;
	}
	/* we have a greate many set of connector, encoder and crtc,
	 * get all the avaliable connector, and we only use the first one */
	if (ret = modeset_setup(fd, res, &info))
		goto err;

	drmModeFreeResources(res);

	return 0;
err:
	return errno;
}

static int modeset_setup(int fd, drmModeRes *res, disp_info *info)
{
	/* we only find the first usable connector, and setup some disp_info */
	int i;
	drmModeConnector *conn;
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
		break;	//finnaly, there is a connector that display
	}
	if (!conn) {
		fprintf(stderr, "Cannot retrieve DRM connector %u:%u (%d): %m\n",
			i, res->connectors[i], errno);
		goto err_con;
	}
	memcpy(&info->mode, conn->modes[0], sizeof(info->mode));
	//we need to setup crtc for current node


err_con:
	return errno;
}
