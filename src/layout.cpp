#include <wlc/wlc.h>
#include <types.h>
#include "handlers.h"
#include "layout.h"
#include "wm.h"
////////////////////////////////////////////////////////////////////////////////
///////////////////////////builtin static functions/////////////////////////////
////////////////////////////////////////////////////////////////////////////////


//TODO remove this!!!!
#include <stdio.h>
extern FILE* debug_file;
//TODO remove this!!!!

/**
 * 
 * @brief the simple floating windows relayout strategy, which is actually
 * doing nothing.
 *
 * Tested
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
		g.size.w = (g.size.w > 0) ? g.size.w : 100;//give a initial wpidth if not avaliable
		
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
	fprintf(debug_file, "master-layout 0\n");
	fflush(debug_file);

	//fprintf(debug_file, "we have %d master, %d minors")
	tw_geometry g = *tw_output_get_geometry(output);
	size_t x, y, w, h;
	x = g.origin.x, y = g.origin.y;
	w = g.size.w,   h = g.size.h;

	//TODO delete this line, as you will need to record the views later
	const tw_handle *views = wlc_output_get_views(output, &nviews);
	
	assert(master_size > 0.0 && master_size < 1.0);
	size_t msize = (size_t)(master_size * ((col_based) ? w : h)); 
	size_t ninstack = nviews - nfloating;

	//the counter records how many windows to rerange this time
	size_t counter = (ninstack > nmaster) ? nmaster : ninstack;
	if (counter == 0)
		return;
	if (col_based) {
		g.size.w = msize;
		relayout_onecol(views, counter, &g);
	} else {
		g.size.h = msize;
		relayout_onerow(views, counter, &g);
	}
	fprintf(debug_file, "master-layout 1\n");
	fflush(debug_file);

        //arrange the minor windows
	if (ninstack - nmaster == 0)
		return;
	if (col_based) {
		g.origin.x = x+msize;
		relayout_onecol(views + counter, ninstack-counter, &g);
	} else {
		g.origin.y = y+msize;
		relayout_onerow(views + counter, ninstack-counter, &g);
	}
	fprintf(debug_file, "master-layout 2\n");
	fflush(debug_file);
	
	//arrange the floating windows
	relayout_float(views + ninstack, nfloating,
		       tw_output_get_geometry(output));

	fprintf(debug_file, "master-layout 3\n");
	fflush(debug_file);
	
}

void
FloatingLayout::relayout(tw_handle output)
{
	//this is not a good method, which only works on on layout cases
	size_t memb;
	const tw_handle* views = wlc_output_get_views(output, &memb);

	fprintf(debug_file, "this is floating relayout\n");
	fflush(debug_file);
	relayout_float(views, memb, tw_output_get_geometry(output));
}



static inline Layout*
tw_output_get_current_layout(wlc_handle output)
{
	struct tw_monitor *mon = (struct tw_monitor *)
		wlc_handle_get_user_data(output);
	if (!mon) {
		fprintf(debug_file, "no data for me???\n ");
		fflush(debug_file);
		return NULL;
	}
	
	return mon->layouts[mon->lays_recent[0]];
}

static inline Layout*
tw_output_get_last_layout(wlc_handle output)
{
	struct tw_monitor *mon = (struct tw_monitor *)
		wlc_handle_get_user_data(output);
	if (!mon)
		return NULL;
	return mon->layouts[mon->lays_recent[1]];
}



//the global layout method
void relayout(tw_handle output)
{
	fprintf(debug_file, "relayouting\n");
	fflush(debug_file);
	//lookup the the current, it may
	Layout *current = tw_output_get_current_layout(output);
	fprintf(debug_file, "this is global relayout\n");
	fflush(debug_file);
	//FIXME looks like my layout classes didn't work
	current->relayout(output);
	fprintf(debug_file, "done relayouting\n");
	fflush(debug_file);
}
