#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface wl_output_interface;
extern const struct wl_interface wl_surface_interface;

static const struct wl_interface *types[] = {
	&wl_output_interface,
	&wl_surface_interface,
};

static const struct wl_message background_manger_requests[] = {
	{ "set_background", "oo", types + 0 },
};

WL_EXPORT const struct wl_interface background_manger_interface = {
	"background_manger", 1,
	1, background_manger_requests,
	0, NULL,
};

