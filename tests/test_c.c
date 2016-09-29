#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct tw_darray {
	void *data;
	size_t esize; //element size
	
	size_t alloc; //allocated size
	size_t header;//the first element

	size_t size;  //the actual size
} tw_darray;

//this will be left value, not assignable.
/* somehow can be done with at, expression is a right value, which is not assignable 
#define tw_darray_at(arr, i, type) ({			\
			size_t _real_loc = ((arr)->header + i) % (arr)->alloc; \
			const type *_mptr = (type *)(arr)->data;	\
			_mptr[_real_loc]; })
*/
bool tw_darray_init(tw_darray *arr)
{
	arr->data = malloc(sizeof(int) * 4);
	if (!arr->data)
		return false;
	arr->esize  = sizeof(int);
	arr->header = 4 / 2;
	arr->size = 0;
	arr->alloc = 4;
	return true;
}

int main(void)
{
	tw_darray darr;
	tw_darray_init(&darr);

}
