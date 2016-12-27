#include <wayland-util.h>
#include <stdio.h>

struct data {
	int d;
	struct wl_list l;
};

int main(int argc, char **argv)
{
	struct data a0 = {.d = 0};
	struct data a1 = {.d = 1};
	struct data a2 = {.d = 2};
	struct data a3 = {.d = 3};
	wl_list_init(&a0.l);
	wl_list_init(&a1.l);
	wl_list_init(&a2.l);
	wl_list_init(&a3.l);

	//this way, we append a list to a node, so we insert it on the top
	
	//wl_list_insert(&a0.l, &a2.l);
	struct wl_list *link = &a3.l;
	do {
		printf("%d\n", ((struct data *)wl_container_of(link, (struct data *)NULL, l))->d);
		link = link->next;
	} while (link != &a3.l);


	return 0;
}
