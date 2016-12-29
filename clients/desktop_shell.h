#ifndef TW_DESKTOP_SHELL_H
#define TW_DESKTOP_SHELL_H

#include <sys/mman.h>
#include <wayland-client.h>
#include <wayland-taiwins_shell-client-protocol.h>
#include <types.h>
#include "wayland.h"

struct taiwins_panel;
struct output_elements;

struct tw_geometry {
	int x, y, width, height, stride;
};

//taiwins_shell is a singleton.
struct taiwins_shell *desktop_get_taiwins_shell(void);
void desktop_set_taiwins_shell(struct taiwins_shell *shell);

static inline void tw_set_geometry(struct tw_geometry *geo, int x, int y, int width, int height)
{
	//assume format will be argb8888
	geo->x = x; geo->y = y;
	geo->width = width; geo->height = height;
	geo->stride = 4 * width;
}
static inline int tw_geometry_get_size(struct tw_geometry *geo)
{
	return geo->stride * geo->height;
}

struct static_surface {
	struct output_elements *output;
	enum nonapp_surface_stage type;

	struct tw_geometry geometry;
	int x, y, width, height, format;

	struct wl_surface *wl_surface; //it is bounded to a surface already
	struct nonapp_surface *na_surface;
 	struct wl_buffer *wl_buffer; //this buffer is kepped

	void *data; //mmaped data
};

//but we can only activate one dynamic buffer at a time
struct dynamic_buffer {
	
};



static inline int
taiwins_panel_get_size(void) {return 16 + 4;}; //16px is a good font size, and the 4 is the gap

/**
 * @brief the element that every output has
 *
 * A collection of per output data, it contains (static)panel buffer,
 * (static)background(wallpaper) buffer and a temporary buffer. The good news is
 * that the temporary buffer is always smaller than background buffer
 */
struct output_elements {
	bool initialized;
	tw_list list;
	
	//global info-keep
	struct registry *registry; //so we can access to global
	struct wl_output *output;
//	struct taiwins_shell *shell;

	struct {
		int x, y;
		int px_width, px_height;
		int scale;
		enum wl_output_transform transform;
	} params[2];

	int curr_param; //this is either 0 or 1
	
	int id;
	int physical_width, physical_height; //monitor param, in milli-meter,
					     //you can determine dpi combine
					     //this and resolution

	//wayland shm impl is weird, as use the fd directly from clients
	int fd;
	struct wl_shm_pool *pool;
	struct static_surface wallpaper, panel;
	
};

//this is a horse keeping code, so you don't need to write it like every time
int output_create_static_surface(struct static_surface *sur , struct output_elements *elem,
				  struct tw_geometry *geo, enum nonapp_surface_stage stage, int offset)
{
	struct taiwins_shell *shell = desktop_get_taiwins_shell();
	if (!shell)
		return -1;
	sur->geometry = *geo;
	sur->type = stage;
	sur->wl_surface = wl_compositor_create_surface(elem->registry->compositor);
	//FIXME: here we have a core dump, for sure, we cannot get
	//something. elem->shell is NULL, you gotta have something
	sur->na_surface = taiwins_shell_create_nonapp_surface(desktop_get_taiwins_shell(), elem->output);
	nonapp_surface_registre(sur->na_surface, elem->output, sur->wl_surface,
				sur->geometry.x, sur->geometry.y, sur->geometry.width, sur->geometry.height, stage);

	sur->wl_buffer = wl_shm_pool_create_buffer(elem->pool, offset, sur->geometry.width, sur->geometry.height, sur->geometry.stride,
						   WL_SHM_FORMAT_ARGB8888);
	sur->data = mmap(NULL, tw_geometry_get_size(&(sur->geometry)), PROT_READ | PROT_WRITE, MAP_SHARED, elem->fd, offset);
	return 0;
}






#endif /* EOF */
