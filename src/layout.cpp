#include <wlc/wlc.h>
#include <types.h>
#include "layout.h"

////////////////////////////////////////////////////////////////////////////////
///////////////////////////builtin static functions/////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * 
 * @brief the simple floating windows relayout strategy, which is actually
 * doing nothing.
 */
static void
relayout_float(const tw_handle *views, const size_t nviews,
	       const wlc_geometry *geo)
{
	//this method has to be called in safe env.
	if(!views || !geo)
		return;
	const wlc_size *r = &geo->size;
	//this method is not safe, how do you ensure you will not access
	//unallocated memory? c++ vector?
	for (size_t i = 0; i < nviews; i++) {
		//the only thing we are doing here is ensure the the window
		//does not go out or border, but is may not be good, since
		//windows can have border
		struct wlc_geometry g = *wlc_view_get_geometry(views[i]);
		g.size.h = (g.size.h > 0) ? g.size.h : 100;//give a initial height if not avaliable
		g.size.w = (g.size.w > 0) ? g.size.w : 100;//give a initial width if not avaliable
		
		int32_t view_botton = g.origin.y+g.size.h;
		int32_t view_right = g.origin.x+g.size.w;
		
		g.size.h = MIN(view_botton, r->h) - g.origin.y;
		g.size.w = MIN(view_right, r->w) - g.origin.x;
		wlc_view_set_geometry(views[i], 0, &g);
	}
}

/*
 * @brief range a column of window on the given geometry.
 */
static void
relayout_onecol(const wlc_handle *views, const size_t nviews,
		const struct wlc_geometry *geo)
{
	if (nviews == 0)
		return;
	size_t y = geo->origin.y;
	//if nviews == 0, crashes
	size_t srow = (size_t) (geo->size.h / nviews);
	for (size_t i = 0; i < nviews; i++) {
		struct wlc_geometry g = {
			{geo->origin.x, y},//point
			{geo->size.w, srow}//size
		};
		y += srow;
		wlc_view_set_geometry(views[i], 0, &g);
	}
}

static void
relayout_onerow(const wlc_handle *views, const size_t nviews,
		const struct wlc_geometry *geo)
{
	if (nviews == 0)
		return;
	size_t x = geo->origin.x;
	//if nviews == 0, crashes
	size_t scol = (size_t) (geo->size.w / nviews);
	for (size_t i = 0; i < nviews; i++) {
		struct wlc_geometry g = {
			{x, geo->origin.y},//point
			{scol, geo->size.h}//size
		};
		x += scol;
		wlc_view_set_geometry(views[i], 0, &g);
	}
}


void
MasterLayout::relayout(tw_handle output)
{
	tw_geometry g = *tw_output_get_geometry(output);
	size_t x, y, w, h;
	x = g.origin.x, y = g.origin.y;
	w = g.size.w,   h = g.size.h;
	
	assert(master_size > 0 && master_size < 1.0);
	size_t msize = (size_t)(master_size * ((col_based) ? w : h)); 
	size_t ninstack = nviews - nfloating;
	assert(nfloating <= nviews && nmaster <= ninstack);

	if (col_based) {
		g.size.w = msize;
		 relayout_onecol(views, nmaster, &g);
	} else {
		g.size.h = msize;
		relayout_onerow(views, nmaster, &g);
	}
        //arrange the minor windows
	if (col_based) {
		g.origin.x = x+msize;
		relayout_onecol(views + nmaster, ninstack-nmaster, &g);
	} else {
		g.origin.y = y+msize;
		relayout_onerow(views + nmaster, ninstack-nmaster, &g);
	}
	//arrange the floating windows
	relayout_float(views + ninstack, nfloating,
		       tw_output_get_geometry(output));
}

void
Floating::relayout(tw_handle output)
{
	const struct wlc_size *r;
	if (!(r = wlc_output_get_resolution(output)))
		return;
	size_t memb;
	const tw_handle* views = wlc_output_get_views(output, &memb);
	relayout_float(views, memb, r);
}


//finally
//the global layout method
void relayout(tw_handle output)
{
	//lookup the the current
	Layout *current; //how I am gonna get current layout :p
	current->relayout(output);
}
