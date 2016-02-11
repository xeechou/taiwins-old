#include <cairo/cairo.h>
#include <wayland-client.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static struct wl_display *gdisplay;
static struct wl_registry *gregistry;
static struct wl_compositor *gcompositor;
static struct wl_surface *gsurface;
static void
global_registry_handler(void *data, struct wl_registry *registry, uint32_t id,
			const char *interface, uint32_t version)
{
	if (strcmp(interface, "wl_compositor") == 0)
		gcompositor = wl_registry_bind(gregistry,
					      id,
					      &wl_compositor_interface,
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
	if (gsurface == NULL) {
		fprintf(stderr, "Cant create surface");
		goto exit;
	}
	/* start rendering */
	cairo_surface_t * surface;
	cairo_t *context;

	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 120, 120);
	context = cairo_create(surface);
	cairo_set_source_rgb(context, 1,0,0);
	cairo_rectangle(context, 0.25,0.25,0.5,0.5);
	cairo_fill(context);
	/* end rendering */
exit:
	wl_display_disconnect(gdisplay);
	exit(0);
}
