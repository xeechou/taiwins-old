#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <wlc/wlc.h>
#include <wlc/wlc-wayland.h>
#include <wlc/wlc-render.h>
#include <wayland-server.h>
#include <linux/input.h>
#include <cairo/cairo.h>
#include "server.h"

//wlc_log

void set_background(struct wl_client *client,
		    struct wl_resource *resource,
		    struct wl_resource *output,
		    struct wl_resource *surface)
{
	//they have assert first
	wlc_handle wlc_output = wlc_handle_from_wl_output_resource(output);
	wlc_resource wlc_surface = wlc_resource_from_wl_surface_resource(surface);
	
//	struct wlc_size resolution = *wlc_output_get_resolution(wlc_output);
	wlc_handle_set_user_data(wlc_output, (void *)wlc_surface);
	//wlc_surface_render(wlc_surface, &(struct wlc_geometry){ wlc_origin_zero, resolution});
	//I need more debugging
}


static struct background_manger_interface background_manager_implementation = {
	.set_background = set_background
};

static void bind_background(struct wl_client *client, void *data,
			    unsigned int version, unsigned int id)
{
	struct wl_resource *resource = wl_resource_create(client, &background_manger_interface, version, id);
	wl_resource_set_implementation(resource, &background_manager_implementation, NULL, NULL);
}

void register_background(void)
{
	wl_global_create(wlc_get_wl_display(), &background_manger_interface, 1, NULL, bind_background);
}
