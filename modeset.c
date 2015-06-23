/* This file charges for
 * 1) modesetting
 * 2) manager_display
 */
#include "renderer.h"
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


/* crtc and encoder are bond together, so we have to set them together */
static int init_crtc(int fd, drmModeRes *res,
			    drmModeConnector *conn,
			    disp_info *info, uint32_t location)
{
	memcpy(&info->mode, &conn->modes[0], sizeof(info->mode));
	info->width  = info->mode.hdisplay;
	info->height = info->mode.vdisplay;
	info->location = location;

	/* retrieve a usable encoders */
	drmModeEncoder *enc;
	uint32_t possible_crtcs;

	/* check the current encoder */
	if (conn->encoder_id)
		enc = drmModeGetEncoder(fd, conn->encoder_id);
	if (enc) {
		if (enc->crtc_id)
			info->crtc = enc->crtc_id;
	} else {	//let's hope this branch never get to execute
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



static uint32_t arrange_coordinates(uint32_t last)
{
	//range display, defaultly, we will range display from left to right
	//a more sophisticate approach requires configure information and
	//display information
	//int x = last >> 16;
	//int y = (0x0000ffff) & last;
	//x += 1;
	return (0x00010000) + last;
	//return (x << 16) | (y)	//add a display horizontally
}

/* update display information everytime when assigning a new monitor */
#define X(coor) (coor >> 16)
#define Y(coor) (coor & 0x0000ffff)
static uint32_t update_disp_range(uint32_t max, uint32_t coordinate)
{
	int x = X(coordinate);
	int y = Y(coordinate);
	x = (X(max) > x) ? X(max) : x;
	y = (Y(max) > y) ? Y(max) : y;
	return (x << 16) + y;
}

static int init_connectors(int fd, drmModeRes *res, disp_array *disp_arr)
{

	int i, ret;
	drmModeConnector *conn;
	disp_info *disps = malloc(sizeof(disp_info) * res->count_connectors);
	disp_arr->n_disps = 0;

	uint32_t coordinate = 0;	//location of displays, starts at 0,0
	uint32_t range = 0;

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
		coordinate = arrange_coordinates(coordinate);
		range = update_disp_range(range, coordinate);
		ret = init_crtc(fd, res, conn, &disps[disp_arr->n_disps],
				coordinate);
		if (ret)
			continue;
		disp_arr->n_disps += 1;
		drmModeFreeConnector(conn);
	}
	if (disp_arr->n_disps == 0)
		return -1;
	//This will work, we shrink the size
	disp_arr->disps = realloc(disps, sizeof(disp_arr->n_disps *
							  sizeof(disp_info)));
	// TODO: this function will arrange the displays in the future. Now you
	// only use disp_arr->disps[0]
	disp_arr->range = range;
	return 0;
}


static int init_fb(int fd, drmModeRes *res, disp_info *info)
{
	int ret;

	ret = init_gbm(fd, &info->rh);
	if (ret)
		return ret;
	ret = init_egl(&info->rh, info->width, info->height);
	if (ret)
		return ret;

	return 0;
}

static int destroy_disp(disp_info *info)
{
	destroy_render_handler(&info->rh);
	return 0;
}

int main()
{
	int ret;
	int fd = open("/dev/dri/card0", O_RDWR);
	disp_array displays;
	drmModeRes *res;

	res = drmModeGetResources(fd);
	if(!res) {
		fprintf(stderr, "Failed to get DRM resources (%d)!\n", errno);
		goto err;
	}

	ret = init_connectors(fd, res, &displays);
	if (ret) {
		fprintf(stderr, "Failed in first call\n");
		goto err;
	}
	//fprintf(stdout, "%d\n", displays.n_disps);
	disp_info *info = &displays.disps[0];
	init_fb(fd, res, info);
	destroy_disp(info);

	drmModeFreeResources(res);

	free(displays.disps);

	return 0;
err:
	return errno;
}
