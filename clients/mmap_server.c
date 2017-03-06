#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "buffer.h"

#define MY_SOCK_PATH "/somepath"

int main(void)
{
	int fd = create_buffer(10000);
	if (fd < 0)
		return -1;
	void *buffer = mmap(NULL, 900, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	void *buffer1 = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 4096);
	printf("buffer: %p, buffer1: %p\n, the pagesize is %d, the fd is %d\n", buffer, buffer1, getpagesize(),fd);

//	struct sockaddr my_addr;

	/*
	int socketfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socketfd < 0) {
		printf("socket failed\n");
		return -1;
	}
	memset(&my_addr, '\0', sizeof(my_addr));
	my_addr.sa_family = AF_UNIX;
	bind(socketfd, &my_addr, sizeof(my_addr));
	listen(socketfd, 9000);
	*/
//	while (1)
//		sleep(10);
	//at the same time, we need to tell the client about the fd
	munmap(buffer, 1000);
	munmap(buffer1, 1000);
	close(fd);
}
