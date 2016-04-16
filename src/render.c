
/**
 * @file render.c
 * @brief rendering hooks for compositor
 * 
 * The hooks like pre-render, post-render
 */
#include "handlers.h"
#include <wlc/wlc.h>
#include <wlc/wlc-render.h>


static void output_pre_render(wlc_handle output)
{
	//hand code a wallpapper in the ouput
	//wayland using xml as protocol, it has to be some wayland resource,
	wlc_resource surface;
	if (!(surface = (wlc_resource)wlc_handle_get_user_data(output)))
		return;
	//it works, but my cairo programming skills are ...
	wlc_surface_render(surface, &(struct wlc_geometry) { wlc_origin_zero, *wlc_output_get_resolution(output) } );
}
