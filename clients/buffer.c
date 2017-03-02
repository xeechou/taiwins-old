#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "buffer.h"

//template name, all the shm file name should look like this
static const char template_fname[] = "/TW_SHM_XXXXXX";

//you need file name and then the size, and many things
int create_buffer(int size)
{
	//using the process_id as the fname, is it a good idea?
	//this is safe as 2^32 -1 is 4294967295
	const char *path = getenv("XDG_RUNTIME_DIR");
	char fname[strlen(path) + strlen(template_fname) + 1];
	int fd;

	sprintf(fname, "%s%s", path, template_fname);
#ifdef  HAVE_MKOSTEMP		
	fd = mkostemp(fname, O_CLOEXEC);
	if (fd > 0)
		unlink(fname);
	else
		return fd;
#else
	fd = mkstemp(fname);
	if (fd > 0) {
		unlink(fname);
		long flags = fcntl(fd, F_GETFD);
		if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
			close(fd);
			fd = -1;
		}
	} else
		return fd;
#endif
	if (ftruncate(fd, size) < 0) {
		close(fd);
		return -1;
	}
	return fd;
}


