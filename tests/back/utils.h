#ifndef UTILS_H
#define UTILS_H
#include <sys/types.h>

int set_cloexec_or_close(int fd);
int create_tmpfile_cloexec(char *tmpname);
int create_anonymous_file(off_t size);

#endif /* UTILS_H */
