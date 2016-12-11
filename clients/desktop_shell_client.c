#include <wayland-client.h>
#include "taiwins_shell_client.h"

#include "shell_lib.h"

/** we could try to implement most of things here
 */
int main(int argc, char **argv)
{
	struct wl_display *display = wl_display_connect(NULL);
	if (!display)
		return -1;
	while(1);
	wl_display_disconnect(display);
}
