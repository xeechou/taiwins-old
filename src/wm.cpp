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
#include "layout.h"

enum tw_border_t {
	TW_BORDER_NONE, /* no border, implementation can be intergraded to TW_BORDER_PIX */
	TW_BORDER_PIX,  /* a few pixels as border */
	TW_BORDER_REG   /* a window like border, with title on it? */
};

struct tw_border {
	tw_border_t type; 
	size_t stitle; /* size of title, not used in pixel type */
	size_t sborder; /* the size of top, not used in regular type */

	/* NOTATION: the border should be the same for every output, so stores
	 * the tw_border for every border is really a waste */
};


/**
 * The data that binds to a wlc_view
 */
struct tw_view_data {
	// in relayout method, we always assume the geometry is the content
	// geometry, that is, borders are not included
	
	tw_border *border;
	size_t scale; // windows can have their own scales, for those which
		      // doesn't support HIDPI, default 1.
	tw_size actual; // when the view need to be scaled, we stores the actual size for it
	
};
/**
 * @brief allocate space for the border of a view
 *
 */
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



/**
 *
 * @brief, window manager struct, one per output
 */
struct tw_monitor {
	tw_handle output; ///unique id
	size_t scale; //could be either 1, 2, 3...
	
	//physical attributes
	tw_geometry geometry; // the valid area for create windows.
	//and we have a menu, a very powerful one.
	
	size_t nlayouts;
	//this works for one monitor only, if I have multiple monitor,
	//I should be able to stay on different layout at different monitor
	Layout *layouts; /* array */
	size_t lays_recent[2]; /** recent two layouts */
};

void create_monitor(wlc_handle output)
{
	struct tw_monitor *mon = (struct taiwins_monitor *)malloc(sizeof(struct taiwins_monitor));
	mon->output = output;
	//setup scale based on name.
	wlc_output_get_name(output);
	//get name from config...
	mon->scale = 1;
	
	//setup the geometry, temporary code, chage this later
	mon->geometry.origin = (tw_point){0, 0};
	mon->geometry.size = *wlc_output_get_resolution(output);

	mon->nlayouts = 2;
	mon->layouts = new Layout[mon->nlayouts];

	wlc_handle_set_user_data(output, mon);
}
void destroy_monitor(wlc_handle output)
{
	struct tw_monitor *mon = (struct tw_monitor *)wlc_handle_get_user_data(output);
	delete [] mon->layouts;
}

