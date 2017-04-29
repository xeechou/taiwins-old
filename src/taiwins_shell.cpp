#include <wlc/wlc.h>
#include <wlc/wlc-wayland.h>
#include <wayland-taiwins_shell-server-protocol.h>
#include <wayland-server.h>
#include <types.h>
#include "wm.h"
#include "protocols.h"
#include "debug.h"


//static struct nonapp_surface_interface *nonapp_surface_implementation(void);
//static struct taiwins_shell_interface  *taiwins_shell_implementation(void);


 //we need to rename this data structure, since the work we need is a render surface
struct nonapp_surface {
	tw_list node;
	struct wl_resource *wl_buffer;
	struct wl_resource *surface;
	struct wl_resource *output;
	int type;
};

	//this may not be a good idea.
//struct wl_surface_interface nonapp_surface_impl = {
	
//};
//you probably need to create a draw call for the 	
struct wl_resource *TMP_DATA[3] = {NULL, NULL, NULL};
	
static void
nonapp_surface_registre(struct wl_client *client,
			struct wl_resource *resource,//nonapp_surface_object
			struct wl_resource *output,
			struct wl_resource *surface,
			struct wl_resource *buffer,
			uint32_t type)
{
	assert(type < TWST_NSTAGES);	
	struct nonapp_surface *d = (struct nonapp_surface *)wl_resource_get_user_data(resource);
	d->wl_buffer = buffer;
	d->surface   = surface;
	d->output    = output;

	debug_log("The resource should correspond to create nonappsurface %p\n", resource);

	//This is only a demo show you how to use wl_shm_buffer
	struct wl_shm_buffer *wl_buffer = wl_shm_buffer_get(buffer);
	//I think here is a write protection, 
	wl_shm_buffer_begin_access(wl_buffer);
	void *data = wl_shm_buffer_get_data(wl_buffer);
	data = NULL;
	wl_shm_buffer_end_access(wl_buffer);
	{
		struct tw_monitor *mon = (struct tw_monitor *)
			wlc_handle_get_user_data(wlc_handle_from_wl_output_resource(output));
		mon->static_views[type].wl_surface = surface;
	}
	
	//to the internal compositor, if you change it, you will have big
	//trouble, unless you want to implement your own surface protocol, but I guess you wouldn't succeed
	
	//this gives you 0
//	wlc_handle view = wlc_handle_from_wl_surface_resource(surface);
//	if (!view)
//		debug_log("Oh, no, there is no handle!!!");

	//so here we are tring to get a handle, so we wil know how to range the 
	
//	wlc_view_from_surface(wlc_resource_from_wl_surface_resource(surface),
//			      client,
//			      const struct wl_interface *interface,
//			      const void *implementation,
//			      uint32_t version,
//			      uint32_t id,
//			     void *userdata);
//	if (strcmp((const char *)data, "this is only a test"))
//		debug_log("the data is not correct!!!\n");
	//creating surface
//	wl_resource_set_user_data(resource, NULL);
//	wl_shm_buffer_end_access(wl_buffer);
	
}
	
static void
nonapp_surface_destroy(struct wl_resource *resource)
{
	struct nonapp_surface *data =
		(struct nonapp_surface *)wl_resource_get_user_data(resource);
//	wl_resource_destroy(data->wl_buffer);
	free(data);
}

	
static struct nonapp_surface_interface nonapp_surface_impl = {
	.registre = nonapp_surface_registre
};

	//why don't you put the type data here?
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
	struct nonapp_surface *data = (struct nonapp_surface *)calloc(1, sizeof(*data));
	wl_resource_set_implementation(r, &nonapp_surface_impl, data, nonapp_surface_destroy);
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
	
TW_EXTERNC void
create_taiwins_shell(void)
{
	taiwins_shell_global = wl_global_create(wlc_get_wl_display(), &taiwins_shell_interface, 1, NULL, bind_taiwins_shell);
//	debug_log("taiwins_shell name: %s\n", taiwins_shell_interface.name);
}
	
TW_EXTERNC void
destroy_taiwins_shell(void)
{
	wl_global_destroy(taiwins_shell_global);
	taiwins_shell_global = NULL;
}
	
//now we need to annouce a global, 
