#ifndef TW_PROTOCOL_H
#define TW_PROTOCOL_H

#include <wlc/wlc.h>
#include <wayland-server.h>
#include <types.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

//this is stupid
struct tw_global {
	const struct wl_interface *interface;
	const int version;
	void *data;
	wl_global_bind_func_t bind;
	struct wl_global *global;
};

EXTERNC void tw_globals_registre(void);

EXTERNC void tw_global_unregistre(void);

//taiwins_shell
EXTERNC void create_taiwins_shell(void);
EXTERNC void destroy_taiwins_shell(void);

EXTERNC void create_dummy_shell(void);
EXTERNC void destroy_dummy_shell(void);

#endif /* EOF */
