#include <cairo/cairo.h>
#include <wayland-client.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "../utils.h"


static struct wl_display *gdisplay;
static struct wl_registry *gregistry;
static struct wl_compositor *gcompositor;
static struct wl_surface *gsurface;
static struct wl_shm *gshm;
static struct wl_buffer *gbuffer;
/* we would have more complex structues */
struct window {
	struct wl_surface *surface;
	struct wl_buffer *buffer;
};

static void
global_registry_handler(void *data, struct wl_registry *registry, uint32_t id,
			const char *interface, uint32_t version)
{
	if (strcmp(interface, "wl_compositor") == 0)
		gcompositor = wl_registry_bind(gregistry,
					      id,
					      &wl_compositor_interface,
					      1);
	else if (strcmp(interface, "wl_shm") == 0)
		gshm = wl_registry_bind(gregistry,
					id,
					&wl_shm_interface,
					1);
}



static void
global_registry_remove(void *data, struct wl_registry *registry, uint32_t id)
{
	printf("Got a registry losing event\n");
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
	gbuffer = wl_shm_pool_create_buffer(pool, /*offset*/0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(pool);
	return fd;
}

int main()
{
	gdisplay = wl_display_connect(NULL);
	if (!gdisplay) {
		fprintf(stderr, "you sure you opened a wayland server?");
		return -1;
	}
	gregistry = wl_display_get_registry(gdisplay);
	wl_registry_add_listener(gregistry, &registry_listener, NULL);

	wl_display_dispatch(gdisplay);
	wl_display_roundtrip(gdisplay);

	if (gcompositor == NULL) {
		fprintf(stderr, "Cant find compositor");
		goto exit;
	}
	gsurface = wl_compositor_create_surface(gcompositor);
	//TODO: add gsurface listener for debuging perpose
	if (gsurface == NULL) {
		fprintf(stderr, "Cant create surface");
		goto exit;
	}
	int fd = create_buffer(400,400);
	//wl_buffer_get_set_user_data is not for setting buffer, for storing a
	//struct like(ccontainer_of)


	void *shm_data = mmap(NULL, 400*400*4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	cairo_surface_t * surface;
	cairo_t *context;
	/* start rendering */

	surface = cairo_image_surface_create_for_data(shm_data, CAIRO_FORMAT_ARGB32, 400, 400, 400*4);
	context = cairo_create(surface);
	cairo_set_source_rgb(context, 1,0,0);
	cairo_rectangle(context, 0.25,0.25,0.5,0.5);
	cairo_fill(context);

	/* in the tutorial data was created from mmap, I can also try to do it
	 * with wl_buffer_get_data */
	/*draw on this wl_buffer */

	//wl_surface_damage(gsurface, 0,0, 400,400);
	//int i;
	//uint32_t *pixel = shm_data;
	//for (i = 0; i < 400 * 400; i++)
	//	*pixel++ = 0xffff;

	//now!!! commit the surface
	wl_surface_attach(gsurface, gbuffer, 0,0);
	wl_surface_commit(gsurface);
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
