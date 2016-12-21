#include <wayland-client.h>

#include <wayland-taiwins_shell_dummy-client-protocol.h>
#include <stdio.h>
/** we could try to implement most of things here
 */


//you need to program a wayland-client, register globals, stuff like that

int main(int argc, char **argv)
{
	struct wl_display *display = wl_display_connect(NULL);
	if (!display)
		return -1;

	taiwins_shell_dummpy_call(struct taiwins_shell *taiwins_shell, struct wl_output *output, struct wl_surface *surface);
	//okay, we are connected to wayland servers, we need to draw a interface for ourselves
	wl_display_disconnect(display);
}
