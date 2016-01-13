#include <linux/vt.h>
#include <linux/kd.h>
#include <xf86drm.h>
#include <stdio.h>
#include <unistd.h>

//test if we need root privillege to do Set and Drop Master
int main()
{
	int tty = STDIN_FILENO;
	if (drmSetMaster(tty) < 0)
		fprintf(stderr, "failed to set master\n");
	if (drmDropMaster(tty) < 0)
		fprintf(stderr, "failed to drop master\n");
}
