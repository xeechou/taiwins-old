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
	output->params[0] = (struct output_geometry){0};
	output->params[1] = (struct output_geometry){0};
	output->curr_param = 0;
//	output->params[0].fd = -1;
//	output->params[1].fd = -1;
	output->id = id;
	output->wl_output = wl_output;
	output->registry = reg;
	//this should only get called once
	taiwins_init_output_statics(output);
}

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
	//DEBUG
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
}


static void
output_scale(void *data, struct wl_output *wl_output, int32_t factor)
{
	struct output_elements *output =
		(struct output_elements *)wl_output_get_user_data(wl_output);
	int future_param = 1 - output->curr_param;
	output->params[future_param].scale = factor;
}

/**
 * @brief setup a static surface for an output
 *
 * a static surface has just stage(when to draw) and geometry parameters. here we just announce it
 *
 */
int output_set_static(struct static_surface *sur, struct output_elements *elem,
		  int offset)
{
	struct taiwins_shell *shell = taiwins_get_shell();
	if (!shell)
		return -1;
	//create a place to draw if you don't have one
	if (!sur->na_surface) {
		sur->wl_surface = wl_compositor_create_surface(elem->registry->compositor);
		sur->na_surface = taiwins_shell_create_nonapp_surface(taiwins_get_shell(), elem->wl_output);
	}
	//keep old data here
	int old_fd = sur->fd;
	int old_size = texture_geometry_size(&sur->geometry);
	struct wl_buffer *old_buffer = sur->wl_buffer;
	void *old_data = sur->data;

	int texture_size;
	struct output_geometry *curr_geo = &elem->params[elem->curr_param];
	sur->geometry = texture_geometry_from_ndc(curr_geo,
						  sur->x, sur->y,
						  sur->width, sur->height);
	texture_size = texture_geometry_size(&sur->geometry);
	sur->fd = create_buffer(texture_size);
	{
		struct wl_shm_pool *pool =
			wl_shm_create_pool(elem->registry->shm,
					   sur->fd,
					   texture_geometry_size(&sur->geometry));
		sur->wl_buffer =
			wl_shm_pool_create_buffer(pool, 0,
						  sur->geometry.width, sur->geometry.height,
						  sur->geometry.stride, sur->format);
		wl_shm_pool_destroy(pool);
	}
	//mmap, offset must align the system pagesize
	sur->data = mmap(NULL, texture_size, PROT_READ | PROT_WRITE, MAP_SHARED,
			 sur->fd, 0);
	printf("%d size for mmap, then I got the mmap result %p\n", texture_size, sur->data);
	fflush(stdout);
	if (errno)
		perror("MMAP error occured");
	//I think the use here is simply just announce wl_buffer, since we can
	//get the wl_buffer geometry information directly from wl_buffer, the 4 int is unecessary
	nonapp_surface_registre(sur->na_surface, elem->wl_output, sur->wl_surface, sur->wl_buffer,
				sur->type);
	wl_surface_attach(sur->wl_surface, sur->wl_buffer, 0, 0);
	wl_surface_damage(sur->wl_surface, 0, 0, sur->geometry.width, sur->geometry.height);
	//NOTE: this call doesn't work, version since is larger than surface version.
//	wl_surface_damage_buffer(sur->wl_surface, 0, 0, sur->geometry.width, sur->geometry.height);
	wl_surface_commit(sur->wl_surface);
	//NOTE: we still couldn't get the wlc_view handle out of it but 

	
	//here we have nothing to draw, we could draw something here as well
	//cleanup data
	if (old_fd > 0) {
		wl_buffer_destroy(old_buffer);
		munmap(old_data, old_size);
		close(old_fd);
	}
	return 0;
}

/*
 * @brief output done event
 *
 * setup the output textures
 */
static void
output_done(void *data, struct wl_output *wl_output)
{
	printf("Output done event\n");
	//here is a HACK, all the output needs get registered with
	//taiwins_shell. If this is not the case. We just return and get call
	//again when taiwins_shell regsiters. Or if taiwins_shell registered
	//first. There will be no output in the list. So we get called when output registers.
	struct taiwins_shell *taiwins_shell = taiwins_get_shell();
	if (!taiwins_shell)
		return;
	struct output_elements *output = (struct output_elements *)data;
	output->curr_param = 1 - output->curr_param;
	//get new geometry
	struct output_geometry *old_geometry = &output->params[1 - output->curr_param];
	struct output_geometry *shell_geometry  = &output->params[output->curr_param];
	//no thing changes if output mode stays the same
	if (output_geometry_equal(old_geometry, shell_geometry))
		return;
	//ALLOC and DRAW the statics in for the output
	for (int i = 0; i < ARRAY_SIZE(output->statics); i++)
		output_set_static(&output->statics[i], output, 0);

//	output->initialized = true;

}

static struct wl_output_listener output_listener = {
	.geometry = output_geometry,
	.mode = output_mode,
	.scale = output_scale,
	.done = output_done,
};


static void cleanup_output(struct output_elements *elem)
{
//	if (elem->initialized) {
//		wl_output_release(elem->wl_output);
//		munmap(elem->panel.data, tw_get_geometry_size(&elem->panel.geometry));
//		munmap(elem->wallpaper.data, tw_get_geometry_size(&elem->wallpaper.geometry));
//		close(elem->params[elem->curr_param].fd);
//	}
//	elem->initialized = false;
}

struct desktop_shell {
	struct taiwins_shell *shell;
	tw_list *outputs;

};

//initialization can be done in this, assignment cant
static struct desktop_shell Desktop_shell = {0};

int
taiwins_get_outputs(struct output_elements ***data)
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

void taiwins_add_output(struct output_elements *output)
{
	tw_list_append_elem(&(Desktop_shell.outputs), &output->list);
}

//taiwins_shell is a singleton.
struct taiwins_shell *taiwins_get_shell(void)
{
	return Desktop_shell.shell;
}

void
taiwins_set_shell(struct taiwins_shell *shell)
{
	Desktop_shell.shell = shell;
}

static void
cleanup_taiwins_shell(void)
{
	struct output_elements *tmp;
	tw_list_for_each(tmp, Desktop_shell.outputs, list)
		cleanup_output(tmp);
}

void
taiwins_init_output_statics(struct output_elements *output)
{
	//BG
	output->statics[TWSHELL_BG] = (struct static_surface) {
		.output = output,
		.type   = NONAPP_SURFACE_STAGE_BACKGROUND,
		.x      = 0.0,
		.y      = 0.0,
		.width  = 1.0,
		.height = 1.0,
		.format = WL_SHM_FORMAT_ARGB8888,
		.fd     = -1,
		.wl_surface = NULL,
		.wl_buffer  = NULL,
		.na_surface = NULL,
		.data       = NULL
	};
	//lock
	output->statics[TWSHELL_LOCK] = (struct static_surface) {
		.output = output,
		.type   = NONAPP_SURFACE_STAGE_LOCK,
		.x      = 0.0,
		.y      = 0.0,
		.width  = 1.0,
		.height = 1.0,
		.format = WL_SHM_FORMAT_ARGB8888,
		.fd     = -1,
		.wl_surface = NULL,
		.wl_buffer  = NULL,
		.na_surface = NULL,
		.data       = NULL
	};
	//panel, based on configs
	output->statics[TWSHELL_PANEL] = (struct static_surface) {
		.output = output,
		.type   = NONAPP_SURFACE_STAGE_PANEL,
		.x      = 0.0,
		.y      = 0.975, //(1 - 1.0/40)
		.width  = 1.0,
		.height = 0.025,
		.format = WL_SHM_FORMAT_ARGB8888,
		.fd     = -1,
		.wl_surface = NULL,
		.wl_buffer  = NULL,
		.na_surface = NULL,
		.data       = NULL
	};
}

/*
 * taiwins_shell registering causes output_done callbacks, if there is any.
 */
static void
registre_globals(struct wl_registry *registry,
		      uint32_t id, const char *interface, uint32_t version)
{
	printf("register global %s\n", interface);
	//HACK, see output_done as reference
	if (strcmp(interface, taiwins_shell_interface.name) == 0) {
		taiwins_set_shell(
			(struct taiwins_shell *)wl_registry_bind(registry, id,
								 &taiwins_shell_interface, version));
		 {
			 struct output_elements **outputs;
			 int n_output = taiwins_get_outputs(&outputs);
			 for (int i = 0; i < n_output; i++)
				 output_done(outputs[i], outputs[i]->wl_output);
			 free(outputs);
		 }
	}
	else if (strcmp(interface, wl_output_interface.name) == 0) {
		struct wl_output *wl_output = wl_registry_bind(registry, id, &wl_output_interface, version);
		struct output_elements *output = (struct output_elements *)malloc(sizeof(*output));
		output_init(output, wl_output, id, (struct registry *)wl_registry_get_user_data(registry));
		//haha
		taiwins_add_output(output);
		//add listener also set user_data
		wl_output_add_listener(wl_output, &output_listener, output);
		//garbage collected in the unregistre call
	} 
}

//ref:https://wayland.freedesktop.org/docs/html/apb.html#Client-classwl__event__queue

//You are gonna be the only program that uses desktop_shell protocol, before you
//get any code done, here is some concept you need to know
//wl_event_queue: a lock causes you to sleep.
//wl_display_read: RETURNS(means it will sleep when data is avaliable) when data avaliable
//wl_display_prepare_read(): sleeps until other threads done reading the queue
//wl_display_dispatch(): in this function, those objects' callbacks get called, sleeps if empty queue
//wl_display_dispatch_pending(): finsihing calling callbacks, only it doesn't sleep, if you read, then use this


//so here is the conclusion, if you use multiple queue, you have to use multiple
//thread, there is no other way, all the threads follow
//"prepare-read-dispatch-flush" procedure, so you can create a template from
//that. Otherwise, uses a single queue, the problem is that you can't sleep at somewhere.
int main(int argc, char **argv)
{
	debug_info = fopen("/tmp/deskop_shell_output", "w");
	//remember this will fail

	//you could either write code that 
	struct registry *Registry = client_init(NULL, registre_globals, NULL);
	//TODO:you need to fix this, the cpu usage is too high
	//it shouldn't be like this

/*
	while (wl_display_prepare_read_queue(display, queue) != 0)
		wl_display_dispatch_queue_pending(display, queue);
	wl_display_flush(display);

	ret = poll(fds, nfds, -1);
	if (has_error(ret))
		wl_display_cancel_read(display);
	else
		wl_display_read_events(display);

	wl_display_dispatch_queue_pending(display, queue);
*/
	
	
	while (wl_display_dispatch(Registry->display) != -1) {
		//so the logic here: After you you set up the surface, you need
		//to know when and where to apply for requests.

		//read a list of doing things
	}


	cleanup_taiwins_shell();
	client_finalize(Registry);
	fclose(debug_info);
}
