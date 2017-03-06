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




/*
	size_t panel_height = shell_geometry->scale * taiwins_panel_get_size();
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
	*/
