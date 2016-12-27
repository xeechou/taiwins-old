#include <stdio.h>

#include <types.h>

void copy_int(void *dest, void *src)
{
	*((int *)dest) = *((int *)src);
}

int
main(int argc, char **argv)
{
	tw_array arr = {
		.data = NULL,
		.esize = sizeof(int),
		.alloc = 0,
		.size = 0,
		.copy_elem = copy_int,
	};
	for (int i = 0; i < 100; i++)
		tw_darray_push_back(&arr, &i);
	void *ptr;
	printf("The details of arr: size: %u, esize: %u, alloc: %u\n", arr.size, arr.esize, arr.alloc);
	int val;
	tw_darray_for_each(&arr, val, ptr) {
		printf("%p\t", ptr);
		printf("%d %d\t", *(int*)(ptr), val);
	}
	printf("\n");
}
