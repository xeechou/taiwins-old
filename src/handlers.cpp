#include <unistd.h>
#include "handlers.h"
#include "wm.h"
#include "debug.h"
#include "utils.h"
#include <wlc/wlc.h>
#include <wlc/wlc-wayland.h>
#include <wayland-server.h>

//this is the very shitty code, at least have a command interface, but so far it
//is working, we also need a way to halt all the children
void
compositor_ready_hook(void)
{
	const char *desktop_client = "/home/developer/Projects/taiwins/build/bin/desktop_shell";
	//for now, we just need to start a very simple program
	pid_t pid = fork();
	if (pid == 0) {
		if (execl(desktop_client, desktop_client, NULL) == -1)
			debug_log("file to start the children\n");
	} else 
		return;
}
