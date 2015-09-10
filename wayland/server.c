/** @file server.h
 *  @brief api and private-data for wayland server.
 *  Thif file contains wayland start/end functions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <wayland-server.h>

static struct wl_display *serv_display;
static struct wl_event_loop *serv_event_loop;

/* bind functions follow a resource create, set implemenation procedures.
 * Wayland library implements wl_registry and wl_display for us.
 *
 * On wayland server, when wayland clients call wl_display_connect(), wl_client
 * is created on the server side, by wl_client_create(which will call
 * bind_display internally.
 *
 * display_get_registry is called when a client calls wl_display_get_registry.
 * registry has a bind interface: it is called when wl_registry_bind called.
 */

/** @brief initialize wayland server.
 *
 *  create wayland server/event_loop, and open a socket for clients to connect.
 *  @return false if failed, else true.
 */
bool
serv_initialize(void)
{
	serv_display = wl_display_create();
	if (!serv_display) {
		printf("error creating display\n");
		return false;
	}
	if (wl_display_add_socket(serv_display, NULL) != 0) {
		printf("error creating socket\n");
		return false;
	}
	serv_event_loop = wl_display_get_event_loop(serv_display);
	return true;
}

/** @brief close all resourse for wayland server.
 * */
void
serv_finalize(void)
{
	wl_display_destroy(serv_display);
}

int main()
{
	serv_initialize();
}
