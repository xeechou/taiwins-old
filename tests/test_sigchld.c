#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signalfd.h>


#include <linux/vt.h>
#include <linux/kd.h>
#include <xf86drm.h>


int handle_sigchild(int signo)
{
	printf("handled sigchild.\n");
	return 0;
}
int main()
{
	struct sigaction sa;
}
