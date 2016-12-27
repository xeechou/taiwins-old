#ifndef TW_DESKTOP_SHELL_H
#define TW_DESKTOP_SHELL_H

#include <sys/mman.h>
#include <wayland-client.h>
#include <wayland-taiwins_shell-client-protocol.h>

struct taiwins_panel;
struct output_elements;

struct tw_geometry {
	int x, y, width, height, stride;
};

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

/* here ? */
struct taiwins_panel {
	struct output_elements *output;

	int x, y, width, height; //the coordinate system related to the output
	//mmaped data,
	int format;//the pixel format, usually we just use argb
	
	struct wl_surface *wl_surface;
	struct nonapp_surface *na_surface;
	struct wl_buffer *wl_buffer;
	void *data; //mmaped data, buffer is created based on this
};

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

	//global info-keep
	struct wl_compositor *compositor;
	struct wl_output *output;
	struct taiwins_shell *shell;
	
	int id;
	int x,y; //the start position of the output on a compositor
	int width, height;
	int scale;
	//every output has panel, background to set, it is better to have it
	//here than creating a list of panels or backgrounds

	//wayland shm impl is weird, as use the fd directly from clients
	int fd;
	struct wl_shm_pool *pool;
	struct taiwins_panel panel;
	struct static_surface wallpaper;
	
};

//this is a horse keeping code, so you don't need to write it like every time 
void output_create_static_surface(struct static_surface *sur , struct output_elements *elem,
				  int offset)
{
	sur->wl_surface = wl_compositor_create_surface(elem->compositor);
	sur->na_surface = taiwins_shell_create_nonapp_surface(elem->shell, elem->output);
	nonapp_surface_registre(sur->na_surface, elem->output, sur->wl_surface,
				sur->geometry.x, sur->geometry.y, sur->geometry.width, sur->geometry.height, sur->type);

	sur->wl_buffer = wl_shm_pool_create_buffer(elem->pool, offset, sur->geometry.width, sur->geometry.height, sur->geometry.stride,
						   WL_SHM_FORMAT_ARGB8888);
	sur->data = mmap(NULL, tw_geometry_get_size(&(sur->geometry)), PROT_READ | PROT_WRITE, MAP_SHARED, elem->fd, offset);
}


void output_construct_panel(int weight, int height)
{
	//create a buffer, with the size, usually it will create a anonymose
	//file Then, use this buffer on a cairo handler, and draw something on
	//it. Here user should create the share memory and server only need to use it.
}

#endif /* EOF */
