#ifndef TW_DESKTOP_SHELL_H
#define TW_DESKTOP_SHELL_H

#include <unistd.h>
#include <sys/mman.h>
#include <wayland-client.h>
#include <wayland-taiwins_shell-client-protocol.h>
#include <types.h>
#include "wayland.h"

/* 
 * the reason we want to write the desktop shell is that we want to merge the
 * panel, background and lock togehter into one process, multiple threads,
 * because lock can takeover control of other elements
 * 
 */

struct taiwins_panel;
struct output_elements;

//DESKTOP SHELL, it is just another name for taiwins_shell
//access is via functions.
#ifdef  TWSHELL_NSTATICS
#undef  TWSHELL_NSTATICS
#endif
#define TWSHELL_NSTATICS 3
#define TWSHELL_BG 0
#define TWSHELL_LOCK 1
#define TWSHELL_PANEL 2

struct taiwins_shell *taiwins_get_shell(void);
void taiwins_set_shell(struct taiwins_shell *shell);
void taiwins_add_output(struct output_elements *output);
//this is ugly interface
int taiwins_get_outputs(struct output_elements ***);
void taiwins_init_output_statics(struct output_elements *);

//
struct output_geometry {
	//x,y position within the global compositor space
	int x, y;
	//pixel width of output 
	int px_width, px_height;
	int scale;
	enum wl_output_transform transform;
	//for secure reasons, we use one fd for each surface
//	int fd;
//	struct wl_shm_pool *pool;
};
static inline bool
output_geometry_equal(const struct output_geometry *g0, const struct output_geometry *g1)
{
	return (g0->px_height == g1->px_height) &&
		(g0->px_width == g1->px_width) &&
		(g0->scale == g1->scale);
}

////////geometry code
struct ndc_geometry {
	float x, y, width, height;
};

struct texture_geometry {
	int x, y, width, height, stride;
};

//here the size is aligned to the page size to so mmap will be happy
static inline int texture_geometry_size(const struct texture_geometry *geo)
{
	return getpagesize() * ( (geo->stride * geo->height) / getpagesize() +
				 ((geo->stride * geo->height) % getpagesize() ? 1 : 0));
}

static inline struct texture_geometry
texture_geometry_from_ndc(const struct output_geometry *shell_geo,
	const float x, const float y,
	const float width, const float height)
{
	struct texture_geometry geo = {0};
	geo.x = x * shell_geo->px_width;
	geo.y = y * shell_geo->px_height;
	geo.width = width * shell_geo->px_width;
	geo.height = height * shell_geo->px_height;
	//ARGB=4
	geo.stride = geo.width * 4;
	return geo;
}

////////geometry code
struct static_surface {
	struct output_elements *output;
	enum nonapp_surface_stage type;
	//geometry gets computed based on the output and relative span
	struct texture_geometry geometry;
	//normalized device coordinates
	float x, y, width, height, format;

	//wayland data
	struct wl_surface *wl_surface;
	struct nonapp_surface *na_surface;
	
	//actual data, every buffer gets alloc a new fd, 
	int fd;
 	struct wl_buffer *wl_buffer; //this buffer is kepped
	void *data;
	//as a result, we should have multiple file to write to
	//adding a draw handle here
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
//	bool initialized;
	tw_list list;
	//global info-keep
	struct registry *registry; //so we can access to global
	struct wl_output *wl_output;
	int physical_width, physical_height;	

	struct output_geometry params[2];
	int curr_param; //this is either 0 or 1
	//you actually can get the id from wl_proxy_get_id, you don't need it here
	int id;
	//width and height in millemeters
	struct static_surface statics[TWSHELL_NSTATICS];
	struct static_surface wallpaper, panel;
	//TODO I should also have dynamic buffers for widgets and right click menu,
	//one widgets get activate at a time. So we don't need to worry about
	//the data racing problem
};

#endif /* EOF */
