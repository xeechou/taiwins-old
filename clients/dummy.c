#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-dummy-client-protocol.h>
#include "wayland.h"


struct wl_output *Output;
struct dummy_iface *Dummy;

static void
registre_globals(struct wl_registry *registry,
		      uint32_t id, const char *interface, uint32_t version)
{
	printf("register global %s\n", interface);
	if (strcmp(interface, dummy_iface_interface.name) == 0) {
		Dummy = wl_registry_bind(registry, id, &dummy_iface_interface, version);
	}
	else if (strcmp(interface, wl_output_interface.name) == 0) {
		Output = wl_registry_bind(registry, id, &wl_output_interface, version);
	}
}



int main(int argc, char **argv)
{
	struct registry *reg = client_init(NULL, registre_globals, NULL);
	printf("this is not possible, but display is %p, ", reg->display);
	printf("Output is %p, ", Output);
	printf("Dummy is %p\n", Dummy);
	
	do {
//		sleep(5);
		dummy_iface_dummy_print(Dummy, Output, "this is a joke");
		struct subdummy *subdummy = dummy_iface_create_subdummy(Dummy);
		subdummy_subdummy_print(subdummy, Output, "another joke");
		subdummy_destroy(subdummy);
		printf("lalalalala\n");		
	}
	while (wl_display_dispatch(reg->display) != -1);
	client_finalize(reg);
}
