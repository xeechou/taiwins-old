/**
 * @file wm.c 
 * @brief File implements the window manager features.
 *
 * window manager features include layout structure of a wm, and the 
 * relayout strategy, in modulized structures, thus we are able to 
 * change the strategy later.
 */

#include <wlc/wlc.h>
#include <wlc/geometry.h>
#include <types.h>

class Layout {
protected:
	/* base class */
	tw_handle monitor;//the monitor it belongs to

	//this should be a circle array, since I can but 
	size_t nviews;	//all the views for that layout of the monitor
	tw_handle *views; // the view for the given output
	
public:
	/* every sub-class inherit this method */
	virtual void relayout (tw_handle output);
};

/* the pure floating layout, no stacked/titing window exists */
class Floating : Layout {
public:
	void relayout(tw_handle output);
};

/**
 *
 * @brief the classic DWM like layout method, support row-based and 
 * col-based.
 *
 * 
 */
class MasterLayout : Layout {
	//there are two layouts) and the other words, it should run under any
	//resolution
	bool col_based; /* default two column method */
	size_t nmaster;
	double master_size; // from 0 to 1, default 0.5
	//append the titling windows to the left, floating windows to the
	//end
	size_t nfloating;
public:
	void relayout(tw_handle output);
};

void
MasterLayout::relayout(tw_handle output)
{
	tw_geometry g = *tw_output_get_monitor(output);
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

/**
 *
 * @brief, window manager struct, one per output
 */
struct taiwins_monitor {
	tw_handle output; ///unique id
	
	//physical attributes
	uint32_t xo, yo; //left upper corner
	uint32_t width, height; //valid layout space
	
	
	size_t nlayouts;
	//this works for one monitor only, if I have multiple monitor,
	//I should be able to stay on different layout at different monitor
	class Layout *layouts; /* array */
	size_t lays_recent[2]; /** recent two layouts */
	
};


/*  singleton struct? */
struct taiwins_layout {
	// you can chose two kinds of strategy:
	// 1) the internal relayout function applies to single view, in
	// this way, you need to keep the current global layout information
	// in the layout struct. comment: this could be a bad idea as
	// orgnizing single window is trival, calling functions will take up
	// major time.
	// 2) a global relayout method.
	void (*relayout) (const tw_handle *first_view, const size_t nviews);
};


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

static void
relayout_onecol(const tw_handle *views, const size_t nviews,
		const wlc_geometry *geo)
{
	size_t y = geo->origin.y;
	size_t srow = (size_t) (geo->size.h / nviews);
	for (size_t i = 0; i < nviews; i++) {
		struct wlc_geometry g = {
			{geo->origin.x, y},//point
			{geo->size.w, srow}//size
		};
		y += srow;
	}
}
static void
relayout_onerow(const tw_handle *views, const size_t nviews,
		const wlc_geometry *geo)
{
	size_t x = geo->origin.x;
	size_t scol = (size_t) (geo->size.w / nviews);
	for (size_t i = 0; i < nviews; i++) {
		struct wlc_geometry g = {
			{x, geo->origin.y},//point
			{scol, geo->size.h}//size
		};
		x += scol;
	}
}
