/** @file  server.h
 *  @breif prototypes of wayland server related bindings
 */
#ifndef SERVER_H
struct serv_interface
{
	struct wl_display *display;
	struct wl_event_loop *event_loop;
};
extern struct serv_interface server_tw;


#define SERVER_H
#endif /* eof */
