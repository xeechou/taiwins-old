#ifndef TW_TYPES_H
#define TW_TYPES_H
#include <stdlib.h>
#include <stddef.h>
#include <wlc/wlc.h>
#include <wayland-util.h>
typedef wlc_handle tw_handle;
typedef struct wlc_size  tw_size;
typedef struct wlc_point tw_point;
typedef struct wlc_geometry tw_geometry;

/*
struct iterator{
	void * (*next)(void);
	void * (*prev)(void);
	void * (*begin)(void);
	void * (*end) (void);
	void * data; //I am not sure this would be a good idea.
};
*/

/**
 * @brief tw_array: the only difference for darray is the head is not 0.
 */
typedef struct tw_array {
	void *data;
	size_t esize; //element size
	
	size_t alloc; //allocated size
	size_t header;//the first element
	size_t size;  //the actual size
	
	//these two functions are used for generic programming
	//copy_elem follows dst, src style
	void (*copy_elem) (void*, void *);
	int (*compare) (void *, void *);
} tw_array;

bool tw_array_init(tw_array *arr, size_t esize,
		    void (*copy_elem) (void*, void *),
		    int (*compare)(void *, void *));

const void *tw_array_at(tw_array* arr, size_t i);
void *tw_array_at_unsafe(tw_array *arr, size_t i);
void tw_array_push_back(tw_array*, void*);
bool tw_array_swap(tw_array*, size_t locl, size_t locr);

#define tw_array_for_each(arr, ptr)		\
	for (ptr = (arr)->data;						\
	     (const char *)ptr < ( (const char *)(arr)->data + (arr)->esize * (arr)->size); \
	     (ptr) = (arr)->esize + (char *)(ptr))

//maybe we should give it a for_each???

/**
 *
 * @brief this is a double-end array data type, that should grow as we expand.
 */
typedef struct tw_array tw_darray;

bool tw_darray_init(tw_darray *arr, size_t esize,
		    void (*copy_elem) (void*, void *),
		    int (*compare)(void *, void *));
const void *tw_darray_at(tw_darray* arr, size_t i);
void *tw_darray_at_unsafe(tw_darray *arr, size_t i);
void tw_darray_push_back(tw_darray*, void*);
void tw_darray_push_front(tw_darray*, void*);
bool tw_darray_swap(tw_darray*, size_t locl, size_t locr);


//maybe one day I can simplify this, rightnow it looks too complex
#define tw_darray_for_each(arr, index, ptr)			       \
	for (index = 0, ptr = (char *)(arr)->data + (arr)->esize * (arr)->header; \
	     index < (arr)->size;					\
	     (ptr) = (char *)(arr)->data + (( ((arr)->header+ (++index)) % (arr)->alloc ) * (arr)->esize) )

/**
 * @brief double-link list
 *
 * quick and nasty
 */
typedef struct wl_list tw_list;
#define tw_container_of(ptr, sample, member) wl_container_of(ptr, sample, member)


/* append @other to @list */
static inline void
tw_list_append_list(tw_list *list, tw_list *other)
{
	if (!other)
		return;
	list->prev->next = other;
	other->prev->next = list;
	tw_list *tmp = list->prev;//when I can remove this line...
	list->prev = other->prev;
	other->prev = tmp;
}

//swap should be safe if one and another are both valid
static inline void
tw_list_swap(tw_list *one, tw_list *another)
{
	tw_list tmp = *one;
	//update one,
	one->next = another->next;
	one->prev = another->prev;
	//update one->prev,	
	one->prev->next = another;
	//update one->next,
	one->next->prev = another;
	//update another,
	another->next = tmp.next;
	another->prev = tmp.prev;
	//update another->prev,	
	another->prev->next = one;
	//update another->next,	
	another->next->prev = one;
}

//This code should be used pretty often when we want to switch views to the front.
//we may want to do something like pop-up-update, push-down-update to move the views
static inline void
tw_list_swap_header_update(tw_list **header, tw_list *another)
{
	//always looks for the boundary problems
	if (!(*header)) {
		*header = another;
		return;
	}
	tw_list_swap(*header, another);
	*header = another;
}

static inline void
tw_list_append_elem(tw_list **header, tw_list *elem)
{
	//we somehow assume elem is always valid, header is valid, but *header maynot
	//it will casue problem if header is null
	if (! (*header) ) {
		*header = elem;
		return;
	}
	//append a elem to the the list is actually just insert it before header
	(*header)->prev->next = elem;
	elem->prev = (*header)->prev;
	(*header)->prev = elem;
	elem->next = *header;
}

/* Insert a new header to the tw_list @header */
static inline void
tw_list_insert_header(tw_list **header, tw_list *elem)
{
	tw_list_append_elem(header, elem);
	*header = elem;
}

static inline void
tw_list_remove_update(tw_list **header, tw_list *elem)
{
	//remove one node from the list and update the header if necessary,
	//there could be following boundary situations:
	//0) *elem == header, but not only elem
	//1) *elem is the only elem in the list

	tw_list *prev = elem->prev;
	tw_list *next = elem->next;

	prev->next = next;
	next->prev = prev;
	//case 0: elem == header
	if (elem == *header)
		*header = next;
	//case 1: the last elem
	//there is no assignment to elem directly, so elem->next shouldn't change
	if (elem->next == elem)
		*header = NULL;
}

#define tw_list_insert wl_list_insert
#define tw_list_init  wl_list_init

#define tw_list_length wl_list_length
#define tw_list_empty wl_list_empty
#define tw_list_insert_list wl_list_insert_list

#endif
