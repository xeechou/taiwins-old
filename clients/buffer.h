#ifndef TW_BUFFER_H
#define TW_BUFFER_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>


int create_buffer(int size);

/*
static int buffer_size_1d(size_t width, size_t stride)
{
	return width * stride;
}

static int buffer_size_2d(size_t width, size_t height, size_t stride)
{
	//stride is usually the same size of element size, sometimes bigger, so be aware
	return width * stride * height;
}
*/


#endif /* EOF */
