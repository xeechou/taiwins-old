#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/socket.h>


int main(int argc, char **argv)
{
	int fd = atoi(argv[1]);
	void *mem = mmap(NULL, 1000, PROT_READ, MAP_SHARED, fd, 0);
	if (mem == MAP_FAILED)
		perror("haha, I know it\n");
	while(1);
}
