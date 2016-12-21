#include <string.h>
#include <stdlib.h>
#include <wayland-client.h>

#include "wayland.h"

//There are quite a few things you need to implement here it looks like for one
//type of client, you need to register specific globals for it, I think I can
//add a callback function so the clients can do something specificly

//call wl_registry_add_listener on this
static void
register_globals(void *data, struct wl_registry *registry,
		 uint32_t id, const char *interface, uint32_t version)
{
	struct registry *reg = (struct registry *)data;

	if (strcmp(interface, wl_compositor_interface.name) == 0)
		reg->compositor = (struct wl_compositor *)wl_registry_bind(registry, id, &wl_compositor_interface, version);
	else if (strcmp(interface, wl_shm_interface.name) == 0)
		reg->shm = (struct wl_shm *)wl_registry_bind(registry, id, &wl_shm_interface, version);
	

	//register client specific code
	if (reg->registre)
		reg->registre(registry, id, interface, version);
}


static void
registry_globals_remove(void *data, struct wl_registry *registry, uint32_t name)
{
	//nothing here?
}

static const struct wl_registry_listener registry_listener = {
	.global = register_globals,
	.global_remove = registry_globals_remove
};


/**
 *
 * @brief init a wayland_client, it does all the dirty jobs you don't want to touch
 */
struct registry *
client_init(const char *display_name,
	void (*registre)(struct wl_registry *, uint32_t, const char *, uint32_t))
{
	struct registry *reg = (struct registry *)malloc(sizeof(*reg));
	
	reg->display = wl_display_connect(display_name);
	if (!reg->display)
		goto error;
	struct wl_registry *wayland_registry = wl_display_get_registry(reg->display);
	reg->registre = registre;

	wl_registry_add_listener(wayland_registry, &registry_listener, reg);

	wl_display_dispatch(reg->display);
	wl_display_roundtrip(reg->display);
	
	return reg;
error:
	free(reg);
	return NULL;
}

void client_finalize(struct registry const *reg)
{
	wl_shm_destroy(reg->shm);
	wl_compositor_destroy(reg->compositor);
	wl_display_disconnect(reg->display);
	//the last memory management is users job
}
