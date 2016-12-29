#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <wayland-client.h>
#include <wayland-taiwins_shell-client-protocol.h>
//#include <types.h>
#include "../libs/common/types.h"
/** we could try to implement most of things here
 */
#include "buffer.h"
#include "wayland.h"
#include "desktop_shell.h"


//These are proxy struct, I don't they there is a line like typedef struct wl_proxy taiwins_shell or something;
static struct registry *Registry = NULL; //remmeber this can fail

//if You want to make it a singleton, use a static method
struct wl_output *Output;
struct wl_surface *Surface;

FILE *debug_info;

static void
output_init(struct output_elements *output, int id, struct registry *reg)
{
	output->curr_param = 0;
	output->fd = -1;
	output->id = id;
	output->registry = reg;

	output->initialized = false;
	//output
}

/*******
 * output listeners, why do I put it here?
 *
 */
static void
output_geometry(void *data,
		struct wl_output *wl_output,
		int32_t x, int32_t y,
		int32_t physical_width, int32_t physical_height, int32_t subpixel,
		const char *make, const char *model, int32_t transform)
{
	struct output_elements *output = (struct output_elements *)data;
	int future_param = 1 - output->curr_param;
	output->physical_width = physical_width; output->physical_height = physical_height;
	output->params[future_param].x = x; output->params[future_param].y = y;
}

static void
output_mode(void *data, struct wl_output *wl_output,
	    uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
	//usually it will be both prefered and current
	if (! (flags & WL_OUTPUT_MODE_CURRENT) )
		return; //we only need to care about current mode
	
	struct output_elements *output = (struct output_elements *)data;
	int future_param = 1 - output->curr_param;
	output->params[future_param].px_width = width;
	output->params[future_param].px_height = height;
}



static void
output_scale(void *data, struct wl_output *wl_output, int32_t factor)
{
	struct output_elements *output = (struct output_elements *)data;
	int future_param = 1 - output->curr_param;	
	output->params[future_param].scale = factor;
}

static void
output_done(void *data, struct wl_output *wl_output)
{
	struct output_elements *output = (struct output_elements *)data;
	//now we swith the parameters
	output->curr_param = 1 - output->curr_param;
	//

	struct taiwins_shell *taiwins_shell;
	/*
	size_t buffer_size, panel_height = output->scale * taiwins_panel_get_size();
	struct tw_geometry geo;
	
	tw_set_geometry(&geo, 0, physical_height - panel_height,
			physical_width, panel_height); //for panel
	buffer_size = tw_geometry_get_size(&geo);
	buffer_size += 2 * output->width * output->height * 4;//ba ckground buffer + ddynamic buffer
	//we have a problem here, since we want allow user to configure where panel locate
	output->fd = create_buffer(buffer_size);

	output->pool = wl_shm_create_pool(output->registry->shm, output->fd, buffer_size);
	//for panel
	output_create_static_surface(&output->panel, output, &geo, NONAPP_SURFACE_STAGE_TOP_LAYER, 0);
	//for background
	tw_set_geometry(&geo, 0, 0, physical_width, physical_height);
	output_create_static_surface(&output->wallpaper, output, &geo, NONAPP_SURFACE_STAGE_BUTTON_LAYER,
				     tw_geometry_get_size(&(output->panel.geometry)));
	
	//dynamic buffer, youd don't need to create buffer now, creat it when you need it
	//wl_surface_attach(surface, buffer, 0, 0);
	//wl_surface_damage(surface, 0, 0, physical_width, (output->scale * taiwins_panel_get_size()));
	//wl_surface_commit(surface); if we go here we will successed
	//then again, the same tech applies on wallpaper
	*/
	output->initialized = true;

}

static struct wl_output_listener output_listener = {
	.geometry = output_geometry,
	.mode = output_mode,
	.scale = output_scale,
	.done = output_done,
};



//////registre/unregistre
static void
registre_globals(struct wl_registry *registry,
		      uint32_t id, const char *interface, uint32_t version)
{
	//I didn't receive the 
	fprintf(debug_info, "register global %s\n", interface);
	fflush(debug_info);
	if (strcmp(interface, taiwins_shell_interface.name) == 0) {
		 desktop_set_taiwins_shell(
			 (struct taiwins_shell *)wl_registry_bind(registry, id,
								  &taiwins_shell_interface, version));
		 //TODO: I could manually call output done here
	}
	//there could be multiple surface and outputs
	//there could be more code, but now it is all dummy
	else if (strcmp(interface, wl_output_interface.name) == 0) {
		Output = (struct wl_output *)wl_registry_bind(registry, id, &wl_output_interface, version);
		struct output_elements *output = (struct output_elements *)malloc(sizeof(*output));
		output_init(output, id, (struct registry *)wl_registry_get_user_data(registry));
		//add listener also set user_data
		wl_output_add_listener(Output, &output_listener, output);
		//garbage collected in the unregistre call
	}
	else if (strcmp(interface, wl_surface_interface.name) == 0)
		Surface = (struct wl_surface *)wl_registry_bind(registry, id, &wl_surface_interface, version);
}

static void cleanup_output(struct output_elements *elem)
{
	if (elem->initialized) {
		wl_output_release(elem->output);
		munmap(elem->panel.data, tw_geometry_get_size(&elem->panel.geometry));
		munmap(elem->wallpaper.data, tw_geometry_get_size(&elem->wallpaper.geometry));
		close(elem->fd);
	}
	elem->initialized = false;
	
}

static struct desktop_shell {
	struct taiwins_shell *shell;
	tw_list *outputs;

} Desktop_shell = {0};

void desktop_add_output(struct output_elements *output)
{
	tw_list_append_elem(&(Desktop_shell.outputs), &output->list);
}
//taiwins_shell is a singleton.
struct taiwins_shell *desktop_get_taiwins_shell(void)
{
	return Desktop_shell.shell;
}
void desktop_set_taiwins_shell(struct taiwins_shell *shell)
{
	Desktop_shell.shell = shell;
}

static void cleanup_taiwins_shell(void)
{
	struct output_elements *tmp;
	tw_list_for_each(tmp, Desktop_shell.outputs, list)
		cleanup_output(tmp);
}


//you need to program a wayland-client, register globals, stuff like that
int main(int argc, char **argv)
{
	debug_info = fopen("/tmp/deskop_shell_output", "w");
	//remember this will fail
	Registry = client_init(NULL, registre_globals, NULL);

	//you need to fix this, the cpu usage is too high
	while (wl_display_dispatch(Registry->display) != -1) {
//		taiwins_shell_dummpy_call(Taiwins_shell, Output, Surface);
	}
	cleanup_taiwins_shell();
	client_finalize(Registry);
	fclose(debug_info);
}
