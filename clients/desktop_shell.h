#ifndef TW_DESKTOP_SHELL_H
#define TW_DESKTOP_SHELL_H

#include <unistd.h>
#include <sys/mman.h>
#include <wayland-client.h>
#include <wayland-taiwins_shell-client-protocol.h>
#include <types.h>
#include "wayland.h"

struct taiwins_panel;
struct output_elements;


//desktop shell functions, you only have one desktop shell at a time, so all the
//access is via functions.
struct taiwins_shell *desktop_get_taiwins_shell(void);
void desktop_set_taiwins_shell(struct taiwins_shell *shell);
void desktop_add_output(struct output_elements *output);
int desktop_get_outputs(struct output_elements ***);



////////geometry code
struct geometry {
	int x, y, width, height, stride;
};

static inline void tw_set_geometry(struct geometry *geo, int x, int y, int width, int height)
{
	//assume format will be argb8888
	geo->x = x; geo->y = y;
	geo->width = width; geo->height = height;
	geo->stride = 4 * width;
}
//here the size is aligned to the page size to so mmap will be happy
static inline int tw_get_geometry_size(struct geometry *geo)
{
	return getpagesize() * ((geo->stride * geo->height) / getpagesize() + 1);
}
////////geometry code

struct static_surface {
	struct output_elements *output;
	enum nonapp_surface_stage type;

	struct geometry geometry;
	int x, y, width, height, format;

	//upon updating, you don't need to change re-alloc wl_surface or
	//na_surface, but you need to destroy old_buffer
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


struct twshell_geometry {
	int x, y, px_width, px_height;
	int scale;
	enum wl_output_transform transform;
	//for secure reasons, we use one fd for each surface
	int fd; struct wl_shm_pool *pool;
};

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
	struct wl_output *wl_output;
//	struct taiwins_shell *shell;
	struct twshell_geometry params[2];
	int curr_param; //this is either 0 or 1
	//an object id used in clean object
	int id; 
	int physical_width, physical_height; //monitor param, in milli-meter,
					     //you can determine dpi combine
					     //this and resolution
	//wayland shm impl is weird, as use the fd directly from clients
	struct static_surface wallpaper, panel;
};


#endif /* EOF */
