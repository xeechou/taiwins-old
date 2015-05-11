#include <stdio.h>
#include <stdlib.h>
#include <wayland-client.h>
#include <wayland-server.h>

struct wl_display *display = NULL;

int main(int argc, char **argv)
{
    display = wl_display_create();
    if (display == NULL) {
	fprintf(stderr, "Can't connect to display\n");
	exit(1);
    }
    struct wl_display *client_display = wl_display_connect(NULL);
    //okay, this is not how it works
    if (client_display == NULL)
	printf("failed connected to display\n");

    exit(0);
}
