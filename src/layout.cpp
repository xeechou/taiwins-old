#include <wlc/wlc.h>
#include <types.h>
#include "handlers.h"
#include "wm.h"
#include "utils.h"
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
		fprintf(debug_file, "the geometry for %d view is: x %d, y %d, w %d, h %d\n",
			i,
			g.origin.x,
			g.origin.y,
			g.size.w,
			g.size.h);
		fflush(debug_file);
		
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

///////////////////////////////////////////////////////////////////////////
///////////////////////////////Layout//////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int
Layout::getViewLoc(const tw_handle view)
{
	for(int i = 0; i < this->nviews;i++) {
		if (views[i] == view)
			return i;
	}
	return -1;
}
const tw_handle
Layout::getViewOffset(const tw_handle view, int offset)
{
	int ind = getViewLoc(view);
	return (ind+offset >= 0 && ind+view < nviews) ? views[ind+offset] : 0;
}
///////////////////////////////////////////////////////////////////////////
////////////////////////////Layout Ends////////////////////////////////////
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
///////////////////////////Master Layout///////////////////////////////////
///////////////////////////////////////////////////////////////////////////


int MasterLayout::getViewLoc(const tw_handle view)
{
	tw_list *l = header;
	for (int i = 0; i < nviews; i++) {
		const tw_handle v = ((view_list *)tw_container_of(l, (view_list *)0, link))->view;
		if (view == v)
			return i;
		l=l->next;
	}
	return -1;
}

const tw_handle
MasterLayout::getViewOffset(const tw_handle view, int offset)
{
	//a lot easier :-D
	this->views = getvarr();
	//FIXME It doesn't work to master layout, why???
	const tw_handle v = Layout::getViewOffset(view, -offset);
	rmvarr();
	return v;
}


/**
 * @brief create a array of current views.
 * this code was buggy before, you need to test it somehow
 */
tw_handle *
MasterLayout::getvarr(void)
{
	this->views = (tw_handle *)malloc(this->nviews * sizeof(tw_handle));
	tw_list *l = this->header;
	for (int i = 0; i < this->nviews; i++) {
		this->views[i] = ((view_list *)tw_container_of(l, (view_list *)0, link))->view;
		l = l->next;
	}
	return this->views;
}


/**
 * @brief rm this array after you done relayout.
 */
void MasterLayout::rmvarr(void)
{
	free(this->views);
}

bool MasterLayout::createView(tw_handle view)
{
	//just create a view, nothing more
	//you don't really care the output
	struct tw_view_data *view_data = (struct tw_view_data *)malloc(sizeof(struct tw_view_data));
	if (!view_data)
		return false;
	
	//processing link
	view_data->link.l.view = view;
	tw_list *l = &(view_data->link.l.link);
	
	debug_log("master::createView\n");
	//handle link
	tw_list_init(l);
	//append a list to a node make it the header of link
	tw_list_insert_header(&(this->header), l);

	//update border
	view_data->border = &((struct tw_monitor *)
			      wlc_handle_get_user_data(this->monitor))->border;
	
	wlc_handle_set_user_data(view, view_data);
	debug_log("done master::createView\n");

	//reference account :p ???
	this->nviews++;
	return true;
}



void
MasterLayout::destroyView(tw_handle view)
{
	struct tw_view_data *view_data = (struct tw_view_data *)wlc_handle_get_user_data(view);
	tw_list *l = &(view_data->link.l.link);
	tw_list_remove_update(&(this->header), l);
	
	this->nviews--;
	free(view_data);
}



void
MasterLayout::relayout(tw_handle output)
{
	debug_log("master layout begins\n");

	tw_geometry g = *tw_output_get_geometry(output);
	size_t x, y, w, h;
	x = g.origin.x, y = g.origin.y;
	w = g.size.w,   h = g.size.h;

	//TODO delete this line, as you will need to record the views later
	const tw_handle *views = this->getvarr();//correct, but why???
	
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
	debug_log("master-layout 1\n");

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
	debug_log("master-layout 2\n");
	
	//arrange the floating windows
	relayout_float(views + ninstack, nfloating,
		       tw_output_get_geometry(output));

	debug_log("master-layout 3\n");
	this->rmvarr();
}


///////////////////////////////////////////////////////////////////////////
////////////////////////Master Layout ends/////////////////////////////////
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//////////////////////////Floating Layout//////////////////////////////////
///////////////////////////////////////////////////////////////////////////

bool
FloatingLayout::createView(tw_handle view)
{
	//it really doesn't matter, just put it on the top of the list
	//0) insert at the end of view array.
	//wlc_view_bring_to_front(view)
	//we don't to relayout here
}

void
FloatingLayout::relayout(tw_handle output)
{
	//this is not a good method, which only works on on layout cases
	size_t memb;
	const tw_handle* views = wlc_output_get_views(output, &memb);

	debug_log("this is floating relayout\n");

	relayout_float(views, memb, tw_output_get_geometry(output));
}


///////////////////////////////////////////////////////////////////////////
//////////////////////////Floating Layout//////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static inline Layout*
tw_output_get_current_layout(wlc_handle output)
{
	struct tw_monitor *mon = (struct tw_monitor *)
		wlc_handle_get_user_data(output);
	if (!mon) {
		debug_log("no data for me??\n");
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


static inline Layout*
tw_view_get_layout(tw_handle view)
{
	return tw_output_get_current_layout(wlc_view_get_output(view));
}


/////////////////////////////handles//////////////////////
//the global layout method
void relayout(tw_handle output)
{
	debug_log("relayouting\n");

	Layout *current = tw_output_get_current_layout(output);

	debug_log("this is the global relayout callback\n");

	current->relayout(output);
	
	debug_log("done relayouting\n");
}

bool view_created(tw_handle view)
{
	//routine:
	//0): create a view: which is most likely insert a node to a list
	tw_handle output = wlc_view_get_output(view);
	Layout *layout = tw_output_get_current_layout(output);//it shouldn't be
							      //Null, one output
							      //should at least
							      //has one layout
	if (!layout->createView(view))
		return false;

	//1): setting up the view visibility attributes, focus attributes, etc.
	wlc_view_set_mask(view, wlc_output_get_mask(output));
	wlc_view_bring_to_front(view); //you have to call it for float layout
	wlc_view_focus(view);
	//2): relayout
	relayout(wlc_view_get_output(view));
	return true;

}


void view_destroyed(tw_handle view)
{
	tw_handle output, pview;
	Layout *layout;
	//routine:
	//0): find out the most previous view.
	output = wlc_view_get_output(view);
	layout = tw_output_get_current_layout(output);
	//there are always problems, if we use link list, we may end up find
	//view it self. If we use array, we probably end up get invalid index
	pview = layout->getViewOffset(view, -1);

	layout->destroyView(view);
	wlc_view_focus(pview);
	relayout(wlc_view_get_output(view));

}


