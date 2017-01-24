#include <wayland-server.h>
#include <wlc/wlc-wayland.h>
#include <wayland-dummy-server-protocol.h>
#include "debug.h"
#include "protocols.h"


#ifdef __cplusplus
extern "C" {
#endif

void subdummy_print(struct wl_client *client,
		     struct wl_resource *resource,
		     struct wl_resource *output,
		     const char *string)
{
	debug_log("SUBDUMMY: we got an request: %s\n", string);	
}
	
static struct subdummy_interface subdummy_impl = {
	.subdummy_print = subdummy_print,
};

	
static void
dummy_print(struct wl_client *client,
	    struct wl_resource *resource,
	    struct wl_resource *output,
	    const char *string)
{
	debug_log("DUMMY: we got an request: %s\n", string);
}

void create_subdummy(struct wl_client *client,
		     struct wl_resource *resource,
		     uint32_t id)
{
	struct wl_resource *r = wl_resource_create(client, &subdummy_interface, wl_resource_get_version(resource), id);
	if (!r) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(r, &subdummy_impl, NULL, NULL);
}
	
static struct dummy_iface_interface dummy_iface_impl = {
	.dummy_print = dummy_print,
	.create_subdummy = create_subdummy,
};

static void bind_dummy_shell(struct wl_client *client, void *data,
			       unsigned int version, unsigned int id)
{
	struct wl_resource *r = wl_resource_create(client,
						   &dummy_iface_interface,
						   version, id);

	if (!r) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(r, &dummy_iface_impl, NULL, NULL);
}

	


	
void
create_dummy_shell(void)
{
	wl_global_create(wlc_get_wl_display(), &dummy_iface_interface,
			 1, NULL, bind_dummy_shell);
}

#ifdef __cplusplus
}
#endif
