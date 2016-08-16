/**
 * @file wm.c 
 * @brief File implements the window manager features.
 *
 * window manager features include layout structure of a wm, and the 
 * relayout strategy, in modulized structures, thus we are able to 
 * change the strategy later.
 */
#include <stdlib.h>
#include <wlc/wlc.h>
#include <wlc/geometry.h>
#include <types.h>
#include "wm.h"

/* everyone include this */
#include "handlers.h"

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

//for the debugging purpose
#include <stdio.h>
extern FILE* debug_file;


bool
output_created(wlc_handle output)
{
	fprintf(debug_file, "creating output\n");
	fflush(debug_file);
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

	//TODO:
	//change this could later, introducing a switch
	for (int i = 0; i < mon->nlayouts; i++)
		mon->layouts[i] = new MasterLayout(output);

	wlc_handle_set_user_data(output, mon);
	
	//you forgot to update these two!!!
	mon->lays_recent[0] = 0;
	mon->lays_recent[1] = 0;
	
	fprintf(debug_file, "done creating output\n");
	fflush(debug_file);
	return true;
}

void
output_destroyed(wlc_handle output)
{
	struct tw_monitor *mon = (struct tw_monitor *)wlc_handle_get_user_data(output);
	//deallocate the 
	
	for (int i = 0; i < mon->nlayouts; i++)
		delete mon->layouts[i];
	free(mon);
}

