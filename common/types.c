#include "types.h"

bool
tw_array_init(tw_array *arr, size_t esize,
		    void (*copy_elem) (void*, void *),
		    int (*compare_cb)(void *, void *) )
{
	if (!compare_cb)
		return false;
	arr->copy_elem = copy_elem;
	arr->compare = compare_cb;
	arr->esize = esize;
	arr->size = 0;

	arr->data = malloc(esize * 4);
	if (!arr->data)
		return false;
	arr->alloc = 4;
	//single end array doesn't use header anyway	
	arr->header = 0;
	return true;
}
void
tw_array_destroy(tw_array *arr)
{
	if (arr->data)
		free(arr->data);
}

/*
 * interally grow the tw_array data type, it should be work at any condition
 */
static bool
tw_array_grow(tw_array *arr)
{
	//in case old size is zero
	size_t new_size = (arr->alloc != 0) ? (arr->esize * arr->alloc * 2) : 4 * arr->esize;
	void *new_data = realloc(arr->data, new_size);
	if (!new_data)
		return false;
	arr->data = new_data;
	arr->alloc = (new_size / arr->esize);
	//header is never used
	return true;
}


const void *
tw_array_at(tw_array* arr, size_t i)
{
	if (i > arr->size)
		return NULL;
	typedef char data_t[arr->esize];
	data_t *data = arr->data;
	return &data[i];
}

void *tw_array_at_unsafe(tw_array *arr, size_t i)
{
	typedef char data_t[arr->esize];
	data_t *data = arr->data;
	return &data[i];
}
void tw_array_push_back(tw_array *arr, void *elem)
{
	if (arr->size == arr->alloc)
		tw_array_grow(arr);
	arr->copy_elem(tw_array_at_unsafe(arr, arr->size), elem);
	arr->size++;
}
//bool tw_array_push_front(tw_array*, void*);
bool tw_array_swap(tw_array*, size_t locl, size_t locr);



bool
tw_darray_init(tw_darray *arr, size_t esize,
		    void (*copy_cb) (void*, void *),
		    int (*compare_cb) (void *, void *))
{
	if (!compare_cb)
		return false;
	arr->copy_elem = copy_cb;
	arr->compare = compare_cb;
	arr->esize = esize;
	arr->size = 0;

	arr->data = malloc(esize * 4);
	if (!arr->data)
		return false;
	arr->alloc = 4;
	arr->header = arr->alloc / 2;
	return true;
}

//really, malloc should never fails
static bool
tw_darray_grow(tw_darray *arr)
{
	size_t new_size = (arr->alloc != 0) ? (arr->esize * arr->alloc * 2) : 4 * arr->esize;
	void *new_data = malloc(arr->esize * arr->alloc * 2);
	if (!new_data)
		return false;
	size_t old_size = (arr->esize * arr->size);

	
	int index; void *ptr;
	char *elem_addr = (char *)new_data + (arr->esize * arr->size);
	tw_darray_for_each(arr, index, ptr) {
//		printf("%p\t", ptr);
		arr->copy_elem(elem_addr, ptr);
		elem_addr += arr->esize;
	}

	arr->alloc = new_size / arr->esize;
	arr->header = arr->alloc / 2;
	free(arr->data);
	arr->data = new_data;
//	printf("\talloc: %u\n", arr->alloc);
	return true;
}

//different than tw_array_at
const void *tw_darray_at(tw_darray* arr, size_t i)
{
	if (i > arr->size)
		return NULL;
	return tw_darray_at_unsafe(arr, i);
}

void *tw_darray_at_unsafe(tw_darray* arr, size_t i)
{
	size_t index = (arr->header + i) % arr->alloc;
//	printf("%d ", index);
	typedef char data_t[arr->esize];
	data_t *data = arr->data;
	return &data[index];
}

//indentical code to tw_array_push_back
void tw_darray_push_back(tw_darray *arr, void *elem)
{
	if (arr->size == arr->alloc)
		tw_darray_grow(arr);
//	printf("%d ", arr->size);
	arr->copy_elem(tw_darray_at_unsafe(arr, arr->size), elem);
	arr->size++;
}

//this is very smart code?
void tw_darray_push_front(tw_darray *arr, void *elem)
{
	if (arr->size == arr->alloc)
		tw_darray_grow(arr);
	arr->header--;
	arr->copy_elem(tw_array_at_unsafe(arr, 0), elem);
	arr->size++;
}
bool tw_darray_swap(tw_darray*, size_t locl, size_t locr);


