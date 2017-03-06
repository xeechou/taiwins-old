#include <unistd.h>
#include "handlers.h"
#include "wm.h"
#include "debug.h"
#include "utils.h"
#include <wlc/wlc.h>
#include <wlc/wlc-wayland.h>
#include <wlc/wlc-render.h>
#include <wayland-server.h>


extern struct wl_resource *TMP_DATA[3];
//this is the very shitty code, at least have a command interface, but so far it
//is working, we also need a way to halt all the children
void
compositor_ready_hook(void)
{
	const char *desktop_client = "/home/developer/Projects/taiwins/build/bin/desktop_shell";
	//for now, we just need to start a very simple program
//	pid_t pid = fork();
//	if (pid == 0) {
//		if (execl(desktop_client, desktop_client, NULL) == -1)
//			debug_log("file to start the children\n");
//	} else 
//		return;
}

void
output_pre_render(wlc_handle output)
{
	if (!TMP_DATA[0])
		return;
	wlc_resource surface = wlc_resource_from_wl_surface_resource(TMP_DATA[0]);
	struct wlc_geometry tmp_geometry = {wlc_origin_zero, *wlc_output_get_resolution(output)};
	wlc_surface_render(surface, &tmp_geometry);
	debug_log("PRE_RENDER_CALLBACK: done a pre-render\n");
}
