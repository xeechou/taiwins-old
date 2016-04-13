#include <cairo/cairo.h>
#include <wayland-client.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include "utils.h"
#include "client.h"

static struct wl_display *gdisplay;
static struct wl_registry *gregistry;
static struct wl_compositor *gcompositor;
static struct wl_surface *gsurface;
static struct wl_shm *gshm;
static struct wl_buffer *gbuffer;
static struct wl_output *goutput;
static struct background_manger *gbmanger;

/* we would have more complex structues */
struct window {
	struct wl_surface *surface;
	struct wl_buffer *buffer;
};
struct output_state {
	uint32_t flags;
	int32_t width;
	int32_t height;
};

static void display_handle_mode(void *data, struct wl_output *wl_output,
				uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
	struct output_state *state = data;
	if (flags & WL_OUTPUT_MODE_CURRENT) {
		state->flags = flags;
		state->width = width;
		state->height = height;
	}
}

static void display_handle_geometry(void *data, struct wl_output *wl_output,
			 int32_t x, int32_t y, int32_t physical_width, int32_t physical_height,
			 int32_t subpixel, const char *make, const char *model, int32_t transform) {
	// this space intentionally left blank
}

static void display_handle_done(void *data, struct wl_output *wl_output) {
	// this space intentionally left blank
}

static void display_handle_scale(void *data, struct wl_output *wl_output, int32_t factor) {
	// this space intentionally left blank
}

static const struct wl_output_listener output_listener = {
	.mode = display_handle_mode,
	.geometry = display_handle_geometry,
	.done = display_handle_done,
	.scale = display_handle_scale
};


static void
global_registry_handler(void *data, struct wl_registry *registry, uint32_t id,
			const char *interface, uint32_t version)
{
	printf("got a registry event for %s id %d\n", interface, id);
	
	if (strcmp(interface, "wl_compositor") == 0)
		gcompositor = wl_registry_bind(gregistry,
					       id,
					       &wl_compositor_interface,
					       version);
	else if (strcmp(interface, "wl_shm") == 0)
		gshm = wl_registry_bind(gregistry,
					id,
					&wl_shm_interface,
					version);
	else if (strcmp(interface, wl_output_interface.name) == 0) {
		goutput = wl_registry_bind(gregistry, id, &wl_output_interface, version);
		struct output_state *state = malloc(sizeof(*state));
		wl_output_add_listener(goutput, &output_listener, state);
	} else if (strcmp(interface, background_manger_interface.name) == 0) {
		gbmanger = wl_registry_bind(gregistry, id, &background_manger_interface, version);
	}
}


static void
global_registry_remove(void *data, struct wl_registry *registry, uint32_t id)
{
	printf("Got a registry losing event %d\n", id);
}
static const struct wl_registry_listener registry_listener = {
	global_registry_handler,
	global_registry_remove
};



static int create_buffer(const int width, const int height)
{
	uint32_t stride = width * 4;
	uint32_t size = stride * height;
	int fd;

	fd = create_anonymous_file(size);
	//shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	//if (!shm_data) {
	//	fprintf(stderr,"fail to map a buffer image pixel for cairo\n");
	//	close(fd);
	//	return NULL;
	//}

	struct wl_shm_pool *pool = wl_shm_create_pool(gshm, fd, size);
	gbuffer = wl_shm_pool_create_buffer(pool, /*offset*/0, width, height, stride,  WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(pool);
	return fd;
}

int main()
{
	gdisplay = wl_display_connect(NULL);
	if (!gdisplay) {
		fprintf(stderr, "you sure you opened a wayland server?\n");
		return -1;
	}
	gregistry = wl_display_get_registry(gdisplay);
	wl_registry_add_listener(gregistry, &registry_listener, NULL);

	//the roundtrip should announce most of the global object.
	//including the background object
	wl_display_dispatch(gdisplay);
	wl_display_roundtrip(gdisplay);
	//and of course include wl_output
	struct output_state *output_state = wl_output_get_user_data(goutput);
	
	if (gcompositor == NULL) {
		fprintf(stderr, "Cant find compositor");
		goto exit;
	}
	gsurface = wl_compositor_create_surface(gcompositor);


	fprintf(stderr, "here\n");
	//TODO: add gsurface listener for debuging perpose
	if (gsurface == NULL) {
		fprintf(stderr, "Cant create surface");
		goto exit;
	}
	int fd = create_buffer(output_state->width, output_state->height);
	//wl_buffer_get_set_user_data is not for setting buffer, for storing a
	//struct

	void *shm_data = mmap(NULL, output_state->width * output_state->height*4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
//	memset()
	cairo_surface_t * surface;
	cairo_t *context;
	/* start rendering */
	/*
	surface = cairo_image_surface_create_for_data(shm_data, CAIRO_FORMAT_ARGB32,
						      output_state->width,
						      output_state->height, output_state->width*4);
	context = cairo_create(surface);
	cairo_set_source_rgb(context, 1,0,0);
	cairo_rectangle(context, 0.25,0.25,0.5,0.5);
	cairo_fill(context);
	*/
	/* in the tutorial data was created from mmap, I can also try to do it
	 * with wl_buffer_get_data */
	/*draw on this wl_buffer */


	int i;
	uint32_t *pixel = shm_data;
	for (i = 0; i < output_state->width * output_state->height; i++)
		*pixel++ = 0xffff;

	//now!!! commit the surface
	wl_surface_attach(gsurface, gbuffer, 0,0);
	wl_surface_damage(gsurface, 0,0, output_state->width, output_state->height);
	wl_surface_commit(gsurface);
	background_manger_set_background(gbmanger, goutput, gsurface);
	while (wl_display_dispatch(gdisplay) != -1) {
		wl_surface_attach(gsurface, gbuffer, 0,0);
		wl_surface_commit(gsurface);
	}

	wl_surface_destroy(gsurface);

exit:
	wl_display_disconnect(gdisplay);
	close(fd);
	exit(0);
}
