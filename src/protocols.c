#include "protocols.h"
#include <wlc/wlc.h>
#include <wlc/wlc-wayland.h>
#include <wayland-server.h>
#include <wayland-taiwins_shell-server-protocol.h>
#include "debug.h"


extern struct tw_global taiwins_shell_global;

static struct tw_global *GLOBALS[] = {
	&taiwins_shell_global };

/* This is useless
void
tw_globals_add(struct tw_global *global)
{
	assert(global && global->bind && global->interface);
	tw_list_append_elem(&TW_globals, &global->link);
}
*/

void tw_globals_registre(void)
{
	int i;
	debug_log("%d\n", taiwins_shell_interface.version);
	for (i = 0; i < sizeof(GLOBALS) / sizeof(GLOBALS[0]); i++) {
		GLOBALS[i]->global = wl_global_create(wlc_get_wl_display(),
						     GLOBALS[i]->interface,
						     GLOBALS[i]->version,
						     GLOBALS[i]->data,
						     GLOBALS[i]->bind);
		debug_log("I just created a global %p\n", GLOBALS[i]->global);
	}
}

void tw_global_unregistre(void)
{
	int i;
	for (i = 0; i < sizeof(GLOBALS) / sizeof(struct tw_global); i++)
		wl_global_destroy(GLOBALS[i]->global);
}
