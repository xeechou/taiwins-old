#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
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


//if You want to make it a singleton, use a static method
struct wl_surface *Surface;

FILE *debug_info;

static void
output_init(struct output_elements *output, struct wl_output *wl_output, int id, struct registry *reg)
{
	output->curr_param = 0;
	output->params[0].fd = -1;
	output->params[1].fd = -1;
	output->id = id;
	output->wl_output = wl_output;
	output->registry = reg;
	output->initialized = false;
	//output
}
//FIXME: fix the updating
int output_update_static_surface(struct static_surface *sur, struct output_elements *elem,
				 struct geometry *geo, enum nonapp_surface_stage stage, int offset)
{
	printf("I shouldn't be here\n");
	struct taiwins_shell *shell = desktop_get_taiwins_shell();
	if (!shell)
 		return -1;
	struct twshell_geometry *curr_geometry = &elem->params[elem->curr_param];
	//keep the old data record
//	struct wl_buffer *old_buffer = sur->wl_buffer;
//	void *old_map = sur->data;
//	int old_len   = tw_get_geometry_size(&(sur->geometry));
	
	//use the new geometry for static surface
	sur->geometry = *geo;
	sur->type = stage;
//	sur->wl_buffer = wl_shm_pool_create_buffer(curr_geometry->pool, offset,
//						   sur->geometry.width, sur->geometry.height, sur->geometry.stride,
//						   WL_SHM_FORMAT_ARGB8888);
//	sur->data = mmap(NULL, tw_get_geometry_size(&(sur->geometry)),
//			 PROT_READ | PROT_WRITE, MAP_SHARED, curr_geometry->fd, offset);
	nonapp_surface_registre(sur->na_surface, elem->wl_output, sur->wl_surface, sur->wl_buffer,
				sur->geometry.x, sur->geometry.y, sur->geometry.width, sur->geometry.height, stage);

	//now you can safely ummap the old buffer
//	wl_buffer_destroy(old_buffer);
//	munmap(old_map, old_len);
	return 0;
}

/**
 * @brief create the static surface and setup the geometry parameters
 *
 * a static surface has just stage(when to draw) and geometry parameters. here we just announce it
 *
 */
int output_create_static_surface(struct static_surface *sur, struct output_elements *elem,
				 struct geometry *geo, enum nonapp_surface_stage stage, int offset)
{
	struct taiwins_shell *shell = desktop_get_taiwins_shell();
	if (!shell)
		return -1;
	sur->geometry = *geo;
	sur->type = stage;
	sur->wl_surface = wl_compositor_create_surface(elem->registry->compositor);
	struct twshell_geometry *curr_geometry = &elem->params[elem->curr_param];
	sur->na_surface = taiwins_shell_create_nonapp_surface(desktop_get_taiwins_shell(), elem->wl_output);

	sur->wl_buffer = wl_shm_pool_create_buffer(curr_geometry->pool, offset,
						   sur->geometry.width, sur->geometry.height, sur->geometry.stride,
						   WL_SHM_FORMAT_ARGB8888);
	
	//FIXME:I have a map failed here, it may because the size problem, it must be the size problem
//	printf("Okay, now I have no idea, The size is %d, fd is %d, offset is %d\n", tw_get_geometry_size(&(sur->geometry)),
//	       curr_geometry->fd, offset);
	//mmap's offset has to align to the page size/4096
	fflush(stdout);
	sur->data = mmap(NULL, tw_get_geometry_size(&(sur->geometry)),
			 PROT_READ | PROT_WRITE, MAP_SHARED,
			 curr_geometry->fd, offset);
	printf("%d size for mmap, then I got the mmap result %p\n", tw_get_geometry_size((&sur->geometry)), sur->data);
	fflush(stdout);
	if (errno)
		perror("MMAP error occured");
	//here 
//	if (!sur->data)
//	printf("WTF %d\n", tw_get_geometry_size((&sur->geometry)));
//	strcpy((char *)sur->data, "this is only a test");
	//I should register with wl_shm_buffer, or, because we have a new buffer, we can somehow use it as idenitfier
	nonapp_surface_registre(sur->na_surface, elem->wl_output, sur->wl_surface, sur->wl_buffer,
				sur->geometry.x, sur->geometry.y,
				sur->geometry.width, sur->geometry.height,
				stage);
//	printf("I resigtered a static surface at (%d, %d), width and height are (%d, %d)\n",
//	       sur->geometry.x, sur->geometry.y, geo->width, geo->height);
	return 0;
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
	fprintf (debug_info, "current monitor paramters:%d, %d \n", physical_width, physical_height);
	fflush(debug_info);

}

static void
output_mode(void *data, struct wl_output *wl_output,
	    uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
	//usually it will be both prefered and current
	if (! (flags & WL_OUTPUT_MODE_CURRENT) )
		return; //we only need to care about current mode

	fprintf(debug_info, "current output mode is %d, %d\n", width, height);
	fflush(debug_info);
	struct output_elements *output = (struct output_elements *)data;
	int future_param = 1 - output->curr_param;
	output->params[future_param].px_width = width;
	output->params[future_param].px_height = height;
	//so the size is correct
	//printf("current screen resolution is: (%d, %d) and %d px\n", width, height, width * height);
}


static void
output_scale(void *data, struct wl_output *wl_output, int32_t factor)
{
	struct output_elements *output = wl_output_get_user_data(wl_output);
	int future_param = 1 - output->curr_param;
	output->params[future_param].scale = factor;
}

/**
 * @brief output done event
 *
 * we received a whole set of output parameters, we need to create panel and
 * background surface for it
 */
static void
output_done(void *data, struct wl_output *wl_output)
{
	printf("Output done event\n");
	//output output event
	struct taiwins_shell *taiwins_shell = desktop_get_taiwins_shell();
	if (!taiwins_shell)
		return;
	struct output_elements *output = (struct output_elements *)data;
	//swith the output params
	output->curr_param = 1 - output->curr_param;
	//get new geometry
	struct twshell_geometry *old_geometry = &output->params[1 - output->curr_param];
	struct twshell_geometry *shell_geometry  = &output->params[output->curr_param];
	size_t panel_height = shell_geometry->scale * taiwins_panel_get_size();
	struct geometry geo;
	size_t buffer_size;

	tw_set_geometry(&geo, 0, 0, shell_geometry->px_width, shell_geometry->px_height);
	//the easiest way is to return a size align to 
	buffer_size = 2 * tw_get_geometry_size(&geo);
	fflush(stdout);
	//for panel
	tw_set_geometry(&geo, 0, shell_geometry->px_height - panel_height,
			shell_geometry->px_width, panel_height);
	buffer_size += tw_get_geometry_size(&geo);

	//this is the awkward part, you expose the mmap interface to both sides
	shell_geometry->fd = create_buffer(buffer_size);
	shell_geometry->pool = wl_shm_create_pool(output->registry->shm, shell_geometry->fd, buffer_size);

	if (old_geometry->fd < 0) {
		//panel
		output_create_static_surface(&output->panel, output, &geo, NONAPP_SURFACE_STAGE_TOP_LAYER, 0);
//		printf("The panel size should be %d\n", tw_get_geometry_size(&(output->panel.geometry)));
		//wallpaper
		tw_set_geometry(&geo, 0, 0, shell_geometry->px_width, shell_geometry->px_height);
//		printf("The size of the wallpaper is %d\n", tw_get_geometry_size(&geo)); the size is correct as well
		output_create_static_surface(&output->wallpaper, output, &geo, NONAPP_SURFACE_STAGE_BUTTON_LAYER,
					     tw_get_geometry_size(&(output->panel.geometry)));
//		printf("The panel size should be %d\n", tw_get_geometry_size(&(output->panel.geometry)));
	}
	else {
		//panel
		output_update_static_surface(&output->panel, output, &geo, NONAPP_SURFACE_STAGE_TOP_LAYER, 0);
		//wallpaper
		tw_set_geometry(&geo, 0, 0, shell_geometry->px_width, shell_geometry->px_height);
		output_update_static_surface(&output->wallpaper, output, &geo, NONAPP_SURFACE_STAGE_BUTTON_LAYER,
					     tw_get_geometry_size(&(output->panel.geometry)));
		wl_shm_pool_destroy(old_geometry->pool);
		close(old_geometry->fd);
	}
	
	//dynamic buffer, youd don't need to create buffer now, creat it when you need it
	//wl_surface_attach(surface, buffer, 0, 0);
	//wl_surface_damage(surface, 0, 0, physical_width, (output->scale * taiwins_panel_get_size()));
	//wl_surface_commit(surface); if we go here we will successed
	//then again, the same tech applies on wallpaper
	output->initialized = true;

}

static struct wl_output_listener output_listener = {
	.geometry = output_geometry,
	.mode = output_mode,
	.scale = output_scale,
	.done = output_done,
};



//////enregistre, that's french, registre is not!!!
static void
registre_globals(struct wl_registry *registry,
		      uint32_t id, const char *interface, uint32_t version)
{
	printf("register global %s\n", interface);
	if (strcmp(interface, taiwins_shell_interface.name) == 0) {
		 desktop_set_taiwins_shell(
			 (struct taiwins_shell *)wl_registry_bind(registry, id,
								  &taiwins_shell_interface, version));
		 struct output_elements **outputs;
		 int n_output = desktop_get_outputs(&outputs);
		 for (int i = 0; i < n_output; i++)
			 output_done(outputs[i], outputs[i]->wl_output);
		 free(outputs);
	}
	else if (strcmp(interface, wl_output_interface.name) == 0) {
		struct wl_output *wl_output = wl_registry_bind(registry, id, &wl_output_interface, version);
		struct output_elements *output = (struct output_elements *)malloc(sizeof(*output));
		output_init(output, wl_output, id, (struct registry *)wl_registry_get_user_data(registry));
		desktop_add_output(output);
		//add listener also set user_data
		wl_output_add_listener(wl_output, &output_listener, output);
		//garbage collected in the unregistre call
	} 
}

static void cleanup_output(struct output_elements *elem)
{
	if (elem->initialized) {
		wl_output_release(elem->wl_output);
//		munmap(elem->panel.data, tw_get_geometry_size(&elem->panel.geometry));
//		munmap(elem->wallpaper.data, tw_get_geometry_size(&elem->wallpaper.geometry));
//		close(elem->params[elem->curr_param].fd);
	}
	elem->initialized = false;
}

static struct desktop_shell {
	struct taiwins_shell *shell;
	tw_list *outputs;

} Desktop_shell = {0};

int
desktop_get_outputs(struct output_elements ***data)
{
	int n_outputs = tw_list_length(Desktop_shell.outputs);
	*data = malloc(sizeof(void *) * n_outputs);
	struct output_elements **outputs = *data;
	if (!n_outputs)
		return 0;

	struct output_elements *output;
	tw_list_for_each(output, Desktop_shell.outputs, list) {
		*outputs++ = output;
//		fprintf(debug_info, "output %p\n", *(outputs - 1));
//		fflush(debug_info);
	}
	return n_outputs;
}

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
	struct registry *Registry = client_init(NULL, registre_globals, NULL);

	//you need to fix this, the cpu usage is too high
	while (wl_display_dispatch(Registry->display) != -1);

	cleanup_taiwins_shell();
	client_finalize(Registry);
	fclose(debug_info);
}
