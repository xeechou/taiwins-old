#include <wayland-taiwins_shell-server-protocol.h>
#include <wayland-server.h>

#include "protocols.h"

//this two function could return a static variable
static struct nonapp_surface_interface *nonapp_surface_implementation(void);
//static struct taiwins_shell_interface  *taiwins_shell_implementation(void);


static void taiwins_create_nonapp_surface(struct wl_client *client,
					struct wl_resource *resource,
					struct wl_resource *output,
					uint32_t id)
{
	//the resource is the taiwins_shell
	//you need to create a resource for nonapp_surface
	struct wl_resource *r = wl_resource_create(client, &nonapp_surface_interface,
					    wl_resource_get_version(resource), id);

//	wl_resource_set_implementation(r, nonapp_surface_implementation(), NULL, NULL);
	//wl_resource_set_implementation(r, nonapp_surface_implementation(), userdata, wl_resource_destroy_func_t destroy);
	//it should be it, but some implementation also uses a signal, I am not sure what's that for
}
static void taiwins_update_panel(struct wl_client *client,
			  struct wl_resource *resource,
			  struct wl_resource *output,
			  struct wl_resource *surface)
{
	
}

#ifdef __cplusplus
extern "C" {
#endif 

static struct taiwins_shell_interface *
taiwins_shell_implementation(void)
{
	static struct taiwins_shell_interface tw_shell_impl = {
		.create_nonapp_surface = taiwins_create_nonapp_surface,
		.update_panel = taiwins_update_panel,
	};
	return &tw_shell_impl;
}

static void bind_taiwins_shell(struct wl_client *client, void *data,
			       unsigned int version, unsigned int id)
{
	struct wl_resource *resource = wl_resource_create(client, &taiwins_shell_interface, version, id);
	if (!resource)
		wl_client_post_no_memory(client);
	wl_resource_set_implementation(resource, taiwins_shell_implementation(), data, NULL);
}


struct tw_global taiwins_shell_global = {
	.interface = &taiwins_shell_interface,
	.version   = 1,
	.bind      = bind_taiwins_shell,
};

#ifdef __cplusplus
}
#endif 
//now we need to annouce a global, 
