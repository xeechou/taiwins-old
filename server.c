#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-server.h>


typedef struct tw_server {
	wl_display *display;
	wl_event_loop *events;
} tw_server;


int compositor_init(tw_server *server)
{
	server->display = wl_create_display();
}

int compositor_destroy()
{
}
