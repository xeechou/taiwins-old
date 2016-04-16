#include "utils.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <wlc/wlc.h>


#define OPEN_MODE O_RDWR|O_CREAT|O_SYNC|O_APPEND
#define CREATE_MODE S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP

static const char template_fname[] = "/TW-XXXXXX";
/* temp files */


int
utils_set_file_flags(int fd)
{
	long flags = fcntl(fd, F_GETFD);
	if (flags == -1)
		return -1;
	if (fcntl(fd, F_SETFD, flags | O_CLOEXEC) == -1)
		return -1;
	return 0;
}



int
utils_create_temp_file(char *fname)
{
	int fd;
#ifdef _GNU_SOURCE
	fd = mkostemp(fname, O_CLOEXEC);
#else
	fd = mkstemp(fname);
	if (utils_set_file_flags(fd)) {
		close(fd);
		return -1;
	}
#endif
	return fd;
}

int
utils_create_anonymous_file(off_t size)
{
	int fd;
	char *name;
	char *dir = tmp_dir();

	if (!dir)
		return -1;
	name = malloc(strlen(dir) + sizeof(template_fname)+1);
	strcpy(name, dir);
	strcat(name, template_fname);
	fd = utils_create_temp_file(name);

	if (fd < 0) {
		fd = -1;
		goto out;
	}
	unlink(name);	/* now the file is totally anonymous.*/

	if (ftruncate(fd, size) < 0) {
		close(fd);
		fd = -1;
	}
out:
	free(name);
	return fd;
}


/* only use this function when it comes to servious problem */
void utils_time(char *time_string)
{
	time_t time_now;
	struct tm *t;

	time(&time_now);
	t = localtime(&time_now);
	sprintf(time_string, "%d-%d-%d %d:%d:%d", t->tm_year + 1900, t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}

/*********** logging functions **************/

static FILE *logger_file;

/**
 * @brief set up logger function, the logger_file is always setted
 */
int
logger_setup(const char *fname)
{
	int err = 0;
	if (strcmp(fname, "-") == 0 || strcmp(fname, "stderr") == 0)
		logger_file = stderr;
	else if ( (logger_file = fopen(fname, "w")) == NULL) {
		fprintf(stderr, "failed to open logger file %s, fall back to stderr\n", fname);
		logger_file = stderr;
		err = errno;
	}
	return err;
}

static inline const char*
logger_type(enum wlc_log_type type)
{
	switch (type) {
	case WLC_LOG_INFO:
		return "INFO";
		break;
	case WLC_LOG_WARN:
		return "WARN";
		break;
	case WLC_LOG_ERROR:
		return "ERRO";
		break;
	case WLC_LOG_WAYLAND:
		return "WAYLAND";
		break;
	}
	return "NOTYPE";
}

/**
 * @brief logging ability
 *
 * we need to add 
 */
void
logger(enum wlc_log_type type, const char *str)
{
	fprintf(logger_file, "%s: %s\n", logger_type(type), str);
}

/*
int main(void)
{
	int fd = create_anonymous_file(1000);
	if (fd > 0)
		printf("yew, this works\n");
	close(fd);
}
*/
