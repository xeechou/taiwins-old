#ifndef BACKGROUND_SERVER_PROTOCOL_H
#define BACKGROUND_SERVER_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-server.h"

struct wl_client;
struct wl_resource;

struct background_manger;
struct wl_output;
struct wl_surface;

extern const struct wl_interface background_manger_interface;

/**
 * background_manger - just for setting backgound?
 * @set_background: (none)
 *
 * the dummpy interface just for setting background.
 */
struct background_manger_interface {
	/**
	 * set_background - (none)
	 * @output: (none)
	 * @surface: (none)
	 */
	void (*set_background)(struct wl_client *client,
			       struct wl_resource *resource,
			       struct wl_resource *output,
			       struct wl_resource *surface);
};


#ifdef  __cplusplus
}
#endif

#endif
