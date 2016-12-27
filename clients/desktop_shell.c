#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <wayland-client.h>
#include <wayland-taiwins_shell-client-protocol.h>
/** we could try to implement most of things here
 */
#include "buffer.h"
#include "wayland.h"
#include "desktop_shell.h"

//These are proxy struct, I don't they there is a line like typedef struct wl_proxy taiwins_shell or something;
static struct registry *Registry = NULL; //remmeber this can fail

struct taiwins_shell *Taiwins_shell;
struct wl_output *Output;
struct wl_surface *Surface;


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
	output->x = x; output->y = y;
	output->width = physical_width; output->height = physical_height;

	//an output has panel buffer and wallpaper buffer, maybe other buffers,
	//but they are temporary, we don't know the size of a temp buffer, but
	//they wont exceed the wallpaper size.
	size_t buffer_size = 0;
	buffer_size += 2 * output->width * output->height * 4;//ba ckground buffer + ddynamic buffer
	//we have a problem here, since we want allow user to configure where panel locate
	buffer_size += physical_width * output->scale * taiwins_panel_get_size() * 4;
	output->fd = create_buffer(buffer_size);
	output->pool = wl_shm_create_pool(Registry->shm, output->fd, buffer_size);

	//Maybe I can resue my code here
	{
		//1) create two buffers, one by background, one by panel
		output->panel.wl_surface = wl_compositor_create_surface(Registry->compositor);
		output->panel.na_surface = taiwins_shell_create_nonapp_surface(Taiwins_shell, wl_output);
		nonapp_surface_registre(output->panel.na_surface, wl_output, output->panel.wl_surface,
					0, physical_height - output->scale * taiwins_panel_get_size(),
					physical_width, output->scale * taiwins_panel_get_size(),
					NONAPP_SURFACE_STAGE_TOP_LAYER);

		//background
		output->wallpaper.wl_surface = wl_compositor_create_surface(Registry->compositor);
		output->wallpaper.na_surface = taiwins_shell_create_nonapp_surface(Taiwins_shell, wl_output);
		nonapp_surface_registre(output->wallpaper.na_surface, wl_output, output->wallpaper.wl_surface,
					0, 0, physical_width, physical_height,
					NONAPP_SURFACE_STAGE_BUTTON_LAYER);
	}
	//allocate buffer and map them
	{
		output->wallpaper.wl_buffer = wl_shm_pool_create_buffer(output->pool, 0,
									physical_width, physical_height, 4 * physical_width,
									WL_SHM_FORMAT_ABGR8888);
		output->wallpaper.data = mmap(NULL, output->width * 4 * output->height,
					      PROT_READ | PROT_WRITE, MAP_SHARED, output->fd, 0);
	}
	//panel
	{
		output->panel.wl_buffer = wl_shm_pool_create_buffer(output->pool, 2 * output->width * output->height * 4,
								    physical_width, taiwins_panel_get_size(), 4 * physical_width ,
								    WL_SHM_FORMAT_ARGB8888);
		output->panel.data = mmap(NULL, 2 * output->width * output->scale * taiwins_panel_get_size() * 4,
					  PROT_READ | PROT_WRITE, MAP_SHARED, output->fd, 2* output->width * output->height);
	}
	//dynamic buffer, youd don't need to create buffer now, creat it when you need it
	//wl_surface_attach(surface, buffer, 0, 0);
	//wl_surface_damage(surface, 0, 0, physical_width, (output->scale * taiwins_panel_get_size()));
	//wl_surface_commit(surface);
	//then again, the same tech applies on wallpaper
}


static void
output_scale(void *data, struct wl_output *wl_output, int32_t factor)
{
	struct output_elements *output = (struct output_elements *)data;
	output->scale = factor;
}

static struct wl_output_listener output_listener = {
	.geometry = output_geometry,
	.scale = output_scale,
};



//////registre/unregistre
static void
registre_globals(struct wl_registry *registry,
		      uint32_t id, const char *interface, uint32_t version)
{
	if (strcmp(interface, taiwins_shell_interface.name) == 0)
		Taiwins_shell = (struct taiwins_shell *)wl_registry_bind(registry, id,
									 &taiwins_shell_interface, version);
	//there could be multiple surface and outputs
	//there could be more code, but now it is all dummy
	else if (strcmp(interface, wl_output_interface.name) == 0) {
		Output = (struct wl_output *)wl_registry_bind(registry, id, &wl_output_interface, version);
		struct output_elements *output = (struct output_elements *)malloc(sizeof(*output));
		output->id = id;
		wl_output_add_listener(Output, &output_listener, output);
		//garbage collected in the unregistre call
	}
	else if (strcmp(interface, wl_surface_interface.name) == 0)
		Surface = (struct wl_surface *)wl_registry_bind(registry, id, &wl_surface_interface, version);
}


//you need to program a wayland-client, register globals, stuff like that
int main(int argc, char **argv)
{
	//remember this will fail
	Registry = client_init(NULL, registre_globals, NULL);

	while (1) {
//		taiwins_shell_dummpy_call(Taiwins_shell, Output, Surface);
	}
	//implement that dummy call
	client_finalize(Registry);
}
