#include "tw_utils.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>


#define OPEN_MODE O_RDWR|O_CREAT|O_SYNC|O_APPEND
#define CREATE_MODE S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP

int tw_log_fd;

/* file_ops */
int open_file(char *file_name)
{
	int fd = 0;
	fd = open(file_name, OPEN_MODE, CREATE_MODE);
	if (fd < 0)
		return -1; 
	return fd; 
}

int write_to_file(int fd, char *msg_to_file, unsigned int msg_length)
{
	int ret = 0;
	int already = 0;  

	if (already != msg_length) {
		ret = write(fd, msg_to_file + already, msg_length - already);
		if (ret < 0) {
			return -1; 
		} else {
			already += ret;
		}
	}
	return 0;
}

int read_from_file(int fd, char *msg_from_file, unsigned int msg_length)
{
	/* not for now */
	return 0;
}

/* debug */

int init_debug()
{
	tw_log_fd = open_file(DEBUG_FILE);
	if (tw_log_fd < 0) {
		perror("init_debug error!\n");
		return -1; 
	}   
	return 1;
}

/* time */

void get_time(char *time_string)
{
	char ret = 0;
	time_t time_now;
	struct tm *t; 

	time(&time_now);
	t = localtime(&time_now);
	sprintf(time_string, "%d-%d-%d %d:%d:%d", t->tm_year + 1900, t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}
