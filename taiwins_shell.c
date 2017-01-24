#include <wlc/wlc.h>
#include <wlc/wlc-wayland.h>
#include <wayland-taiwins_shell-server-protocol.h>
#include <wayland-server.h>

#include <types.h>
#include "protocols.h"
#include "debug.h"


//static struct nonapp_surface_interface *nonapp_surface_implementation(void);
//static struct taiwins_shell_interface  *taiwins_shell_implementation(void);
#ifdef __cplusplus
extern "C" {
#endif 


//we need to rename this data structure, since the work we need is a render surface
struct nonapp_surface_data {
	tw_list node;
	struct wl_resource *wl_buffer;
};
	
static void
nonapp_surface_registre(struct wl_client *client,
			struct wl_resource *resource,
			struct wl_resource *output,
			struct wl_resource *surface,
			struct wl_resource *buffer,			
			int32_t x,
			int32_t y, 
			int32_t width,
			int32_t height,
			uint32_t type)
{
	struct nonapp_surface_data *d = (struct nonapp_surface_data *)wl_resource_get_user_data(resource);
	{//destroying the old stuff
		//weird thing here that I don't need to 
		if (d->wl_buffer)
			wl_resource_destroy(d->wl_buffer);
		d->wl_buffer = buffer;
	}

	debug_log("The resource should correspond to create nonappsurface %p\n", resource);
	//here I should identify resource, if it is registered?
//	debug_log("NON-APP SURFACE: register surface starts at (%d, %d)\n", x, y);
//	debug_log("with geometry (%d, %d)\n", width, height);
	struct wl_shm_buffer *wl_buffer = wl_shm_buffer_get(buffer);


	
	wl_shm_buffer_begin_access(wl_buffer);
	void *data = wl_shm_buffer_get_data(wl_buffer);
//	if (strcmp((const char *)data, "this is only a test"))
//		debug_log("the data is not correct!!!\n");
	//creating surface
//	wl_resource_set_user_data(resource, NULL);
	wl_shm_buffer_end_access(wl_buffer);
	
}
	
static void
nonapp_surface_desctroy(struct wl_resource *resource)
{
	struct nonapp_surface_data *data = wl_resource_get_user_data(resource);
	wl_resource_destroy(data->wl_buffer);
	free(data);
}

	
static struct nonapp_surface_interface nonapp_surface_impl = {
	.registre = nonapp_surface_registre
};

	
static void taiwins_create_nonapp_surface(struct wl_client *client,
					  struct wl_resource *resource,
					  struct wl_resource *output,
					  uint32_t id)
{
	struct wl_resource *r = wl_resource_create(client, &nonapp_surface_interface,
						    nonapp_surface_interface.version, id);
	if (!r) {
		wl_client_post_no_memory(client);
		return;
	}
	struct nonapp_surface_data *data = calloc(1, sizeof(*data));
	wl_resource_set_implementation(r, &nonapp_surface_impl, data, nonapp_surface_desctroy);
	//TODO: get the render queue and push everything there
	debug_log("If I succeed, we should have a resource %p\n", r);    	
}

static struct taiwins_shell_interface tw_shell_impl = {
	.create_nonapp_surface = taiwins_create_nonapp_surface,
};

	
static void bind_taiwins_shell(struct wl_client *client, void *data,
			       unsigned int version, unsigned int id)
{
	struct wl_resource *resource = wl_resource_create(client,
							  &taiwins_shell_interface,
							  version,
							  id);
	if (!resource)
		wl_client_post_no_memory(client);
	wl_resource_set_implementation(resource, &tw_shell_impl, data, NULL);
//	debug_log("I am not sure, here we bind for version %d, and client %p\n ", version, client);
}


//this is one way of hard coding
static struct wl_global *taiwins_shell_global;
	
void
create_taiwins_shell(void)
{
	taiwins_shell_global = wl_global_create(wlc_get_wl_display(), &taiwins_shell_interface, 1, NULL, bind_taiwins_shell);
//	debug_log("taiwins_shell name: %s\n", taiwins_shell_interface.name);
}
	
void
destroy_taiwins_shell(void)
{
	wl_global_destroy(taiwins_shell_global);
	taiwins_shell_global = NULL;
}
	
#ifdef __cplusplus
}
#endif 
//now we need to annouce a global, 
