#ifndef TW_WAYLAND_H
#define TW_WAYLAND_H

#include <wayland-client.h>

struct registry {
	struct wl_compositor *compositor;
	struct wl_display *display;
	struct wl_shm *shm; //tous les plus programe de wayland utilise shm
	
	// a client may have a custom registering job, usually this function
	// will be implemented as a static function that manipulate a static
	// global data, this is okay since globals are global to a
	// program.

	//having his, on n'a besoin plus ecris beaucoup code de
	// registre.
	void (*registre)(struct wl_registry *registry,
			 uint32_t id,
			 const char *interface,
			 uint32_t version);
};

struct registry *client_init(const char *display_name,
			       void (*registre)(struct wl_registry *, uint32_t, const char *, uint32_t));

void client_finalize(struct registry const *reg);



#endif
