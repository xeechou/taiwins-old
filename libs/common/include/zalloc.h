#ifndef COMMON_ZALLOC_H
#define COMMON_ZALLOC_H

#include <stdlib.h>

void *zalloc(size_t total_size)
{
	void *ret = calloc(total_size, 1);
	if (!ret && total_size)
		exit(-1);
	return ret;
}


#endif /* EOF */
