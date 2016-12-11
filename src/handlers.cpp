#include <unistd.h>
#include "handlers.h"
#include "wm.h"

#include "utils.h"

void
compositor_ready_hook(void)
{
//	const char *desktop_client = "/home/developer/Projects/taiwins/build/clients/desktop_clients";
	//for now, we just need to start a very simple program
	pid_t pid = fork();
	if ((pid = fork()) == 0) {
//		execl(desktop_client, desktop_client, NULL);
	} else 
		return;
}
