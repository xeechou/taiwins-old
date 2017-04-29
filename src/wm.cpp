/**
 * @file wm.c 
 * @brief File implements the window manager features.
 *
 * window manager features include layout structure of a wm, and the 
 * relayout strategy, in modulized structures, thus we are able to 
 * change the strategy later.
 *
 * TODO: In the end, we need to make this file into a c file, mixing c/c++ is
 * really a bad idea? Or why it is a bad idea?
 */
#include <stdlib.h>
#include <wlc/wlc.h>
#include <wlc/wlc-wayland.h>
#include <wlc/geometry.h>
#include "wm.h"

/* everyone include this */
#include "handlers.h"

extern struct tw_compositor compositor;
/**
 * @brief allocate space for the border of a view
 * 
 * Did I test this? Anyone use this?
 */
/*
static
void adjust_border(tw_handle view)
{
	//border adjust algorithm:
	// 1) find the content geometry
	// 2) scale down the content if necessary and store it in actual size
	// there maybe more sophisticated algorithm later, since we need to do divide later
	const tw_geometry *g = wlc_view_get_geometry(view);
	if (!g)
		return;
	struct tw_view_data *d = (struct tw_view_data *)wlc_handle_get_user_data(view);
	if (!d->scale) //enusure we never crash
		d->scale = 1;
	tw_geometry new_g = *g;
	
	switch (d->border->type) {
	case  TW_BORDER_NONE:
		return;
		break;
	case TW_BORDER_PIX:
		//content_geometry
		new_g.origin.x += d->border->sborder;
		new_g.origin.y += d->border->sborder;
		new_g.size.h -= 2*(d->border->sborder);
		new_g.size.w -= 2*(d->border->sborder);
		wlc_view_set_geometry(view, 0, &new_g);
		//scale down the actual geometry
		d->actual.h = (size_t)(new_g.size.h / d->scale);
		d->actual.w = (size_t)(new_g.size.w / d->scale);
		//wlc_handle_set_user_data(view, d);
		break;
	case TW_BORDER_REG:
		//content_geometry
		new_g.origin.y += d->border->stitle;
		new_g.size.h   -= d->border->stitle;
		wlc_view_set_geometry(view, 0, &new_g);
		//scale down the actual geometry
		d->actual.h = (size_t)(new_g.size.h / d->scale);
		d->actual.w = (size_t)(new_g.size.w / d->scale);
		break;
	}
}
*/

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
/////////////////////Implementing callbacks///////////////////
//////////////////////////////////////////////////////////////
struct tw_surface_node *
is_static_view_for_output(const tw_handle view, tw_handle output)
{
	struct wl_resource *surface = wlc_surface_get_wl_resource(wlc_view_get_surface(view));		
	struct tw_monitor *mon = (struct tw_monitor *)wlc_handle_get_user_data(output);
	for (int i = 0; i < TWST_NSTAGES; i++) {
		struct wl_resource *lsurface = mon->static_views[i].wl_surface;
		tw_handle lview = mon->static_views[i].wlc_view;
		if (lsurface == NULL)
			continue;
		if (lsurface == surface || lview == view)
			return &mon->static_views[i];
	}
	return NULL;
}



//for the debugging purpose
#include "debug.h"
bool
output_created(wlc_handle output)
{
	if (wlc_handle_get_user_data(output)) //if we already have the output
		return true;
	
	debug_log("%s:creating output %d\n", wlc_output_get_name(output), output);
	struct tw_monitor *mon = (struct tw_monitor *)malloc(sizeof(struct tw_monitor));
	mon->output = output;
	//setup scale based on name.
	//wlc_output_get_name(output);
	//get name from config...

	//FIXME!!!!
	mon->scale = 1;
	//setup the geometry, temporary code, chage this later
	mon->geometry.origin = (tw_point){0, 0};
	mon->geometry.size = *wlc_output_get_resolution(output);
	mon->nlayouts = 1;
	mon->layouts = (Layout **)malloc(sizeof(Layout*) * mon->nlayouts);
	//TODO, using other layout
	for (int i = 0; i < mon->nlayouts; i++)
		mon->layouts[i] = new MasterLayout(output);
	for (int i = 0; i < TWST_NSTAGES; i++) {
		//so rightnow it is fine, all view is greater than zero
		mon->static_views[i] = {0};
	}
	wlc_handle_set_user_data(output, mon);
	//you forgot to update these two!!!
	mon->lays_recent[0] = 0;
	mon->lays_recent[1] = 0;
//	tw_list_init(&mon->static_views);
	debug_log("done creating output\n");
	return true;
}

void
output_destroyed(wlc_handle output)
{
	debug_log("closing output\n");
	struct tw_monitor *mon = (struct tw_monitor *)wlc_handle_get_user_data(output);
	//deallocate the 
	
	for (int i = 0; i < mon->nlayouts; i++)
		delete mon->layouts[i];
	free(mon);
	debug_log("done closing output\n");
}

