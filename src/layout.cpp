#include <wlc/wlc.h>
#include <types.h>
#include "handlers.h"
#include "wm.h"
#include "utils.h"
////////////////////////////////////////////////////////////////////////////////
///////////////////////////builtin static functions/////////////////////////////
////////////////////////////////////////////////////////////////////////////////


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
		debug_log("relayout_float\n");
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
		wlc_view_bring_to_front(views[i]);
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
		//fprintf(debug_file, "the geometry for %d view is: x %d, y %d, w %d, h %d\n",
		//	i,
		//	g.origin.x,
		//	g.origin.y,
		//	g.size.w,
		//	g.size.h);
		//fflush(debug_file);
		
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
bool
Layout::update_views(void)
{
//	debug_log("this is called\n");
	//free previous information
	if (views)
		free(views);
	if ( !(views = (tw_handle *)malloc(sizeof(tw_handle) * nviews)) )
		return false;

	tw_list *l = this->header;//header is after deleting to one view
	for (size_t i = 0; i < nviews; i++) {
		views[i] = ((view_list *)tw_container_of(l, (view_list *)0, link))->view;
		l = l->next;
	}
	return true;
}

int
Layout::getViewLoc(const tw_handle view)
{
	for(int i = 0; i < this->nviews; i++) {
		if (views[i] == view)
			return i;
	}
	return -1;
}
const tw_handle
Layout::getViewOffset(const tw_handle view, int offset)
{
	int ind = getViewLoc(view);
	//fprintf(debug_file, "WE WANT TO GET OFFSET %d, nviews: %d, loc: %d \n", offset, nviews, ind);
	//fflush(debug_file);
	return (ind+offset >= 0 && ind+offset < nviews) ? views[ind+offset] : 0;
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
	const tw_handle v = Layout::getViewOffset(view, -offset);
	return v;
}


/**
 * @brief create a array of current views.
 * this code was buggy before, you need to test it somehow
 * now it works. There should be no problems for it right now
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

/** 
 * @brief this layout method only update the internal view data structure, it
 * should not do relayout, Relayout will be done in global handles
 */
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

	//FIXME: we should allow different type of border info
	view_data->border = &((struct tw_monitor *)
			      wlc_handle_get_user_data(this->monitor))->border;
	
	wlc_handle_set_user_data(view, view_data);
	debug_log("done master::createView\n");

	//reference account :p ???
	this->nviews++;
	
	if (!this->update_views()) {
		free(view_data);
		return false;
	}
	return true;
}

/** 
 * @brief this layout method only update the internal view data structure, it
 * should not do relayout, Relayout will be done in global handles
 */
void
MasterLayout::destroyView(tw_handle view)
{
	struct tw_view_data *view_data = (struct tw_view_data *)wlc_handle_get_user_data(view);
	tw_list *l = &(view_data->link.l.link);
	tw_list_remove_update(&(this->header), l);
	debug_log("Master destroy View 1\n");
	this->nviews--;
	free(view_data);
	debug_log("Master destroy View 2\n");
	//call this at last
	update_views();//we have this problem???
	debug_log("Master destroy View 3\n");
}


/**
 * @brief the specific relayout method, only be called in relayout handles
 */ 
void
MasterLayout::relayout(tw_handle output)
{
	debug_log("master layout begins\n");

	tw_geometry g = *tw_output_get_geometry(output);
	size_t x, y, w, h;
	x = g.origin.x, y = g.origin.y;
	w = g.size.w,   h = g.size.h;
//	fprintf(debug_file, "we have %d views to relayout, which is %d\n", wl_list_length(header), nviews);
//	fflush(debug_file);
	//TODO delete this line, as you will need to record the views later
	assert(master_size > 0.0 && master_size < 1.0);
	size_t msize = (size_t)(master_size * ((col_based) ? w : h)); 
	size_t ninstack = nviews - nfloating;
	//the counter records how many windows to rerange this time
	size_t counter = (ninstack > nmaster) ? nmaster : ninstack;
	if (counter == 0)
		return;
	if (col_based) {
		g.size.w = (nviews > nmaster) ? msize : w;
		relayout_onecol(views, counter, &g);
	} else {
		g.size.h = (nviews > nmaster) ? msize : h;
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
	debug_log("Floating::createView\n");	
	//pring the view to the front is the problem of relayout
	struct tw_view_data *view_data = (struct tw_view_data *)malloc(sizeof(struct tw_view_data));
	if (!view_data)
		return false;

	view_data->link.l.view = view;
	tw_list *l = &(view_data->link.l.link);
	tw_list_init(l);
	//this is the only line of code that different than Master::createView
	tw_list_append_elem(&(this->header), l);

	//TODO: we should have a view->getProperty call
	view_data->border = &((struct tw_monitor *)
			      wlc_handle_get_user_data(this->monitor))->border;
	
	wlc_handle_set_user_data(view, view_data);
	debug_log("done Floating::createView\n");

	nviews++;//has to come before update views
	if (!this->update_views()) {
		free(view_data);
		return false;
	}
		

	return true;
}


void
FloatingLayout::destroyView(tw_handle view)
{
	struct tw_view_data *view_data = (struct tw_view_data *)wlc_handle_get_user_data(view);
	tw_list *l = &(view_data->link.l.link);
	tw_list_remove_update(&(this->header), l);
	debug_log("Floating destroy View 1\n");
	this->nviews--;
	free(view_data);
	debug_log("Floating destroy View 2\n");
	//call this at last
	update_views();
	debug_log("Floating destroy View end\n");
	
}
void
FloatingLayout::relayout(tw_handle output)
{
	debug_log("Floating relayout\n");
	debug_log("we have %d views, the header is %p", this->nviews, this->views);
	relayout_float(this->views, this->nviews, tw_output_get_geometry(output));
	debug_log("Done Floating relayout\n");
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
	current->relayout(output);
	debug_log("done relayouting\n");
}


//although
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
	debug_log("starting destroy view\n");
	tw_handle output, pview;
	Layout *layout;
	//routine:
	//0): find out the most previous view, I think i3 has a stack that stores all the views you have
	output = wlc_view_get_output(view);
	layout = tw_output_get_current_layout(output);

	//TODO: we need to come up with a constant value for next view
	pview = layout->getViewOffset(view, 1);//get next view...
	//pview1 = layout->getViewOffset(view, -1), chose one from them
	layout->destroyView(view);//we have problems here!
	wlc_view_focus(pview);
	relayout(wlc_view_get_output(view));
}
