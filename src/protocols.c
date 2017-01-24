#include "protocols.h"
#include <wlc/wlc.h>
#include <wlc/wlc-wayland.h>
#include <wayland-server.h>
#include <wayland-taiwins_shell-server-protocol.h>
#include "debug.h"

#ifdef __cplusplus
extern "C" {
#endif
//here we have problems
void tw_globals_registre(void)
{
	//the first protocol
	create_taiwins_shell();
	create_dummy_shell();
}

void tw_global_unregistre(void)
{
	destroy_taiwins_shell();
}

#ifdef __cplusplus
}
#endif
