#ifndef TW_WM_H
#define TW_WM_H

/**
 *
 * @file wm.h data types for window manager
 * this project is c++ without containers, because I don't wanna cause code blob
 */

#include <stdlib.h>
#include <types.h>

/* types used in this place */
class Layout;
struct tw_monitor;
struct wl_resource;


//TODO you need house keeping, remove those that we don't need
enum tw_border_t {
	TW_BORDER_NONE, /* no border, implementation can be intergraded to TW_BORDER_PIX */
	TW_BORDER_PIX,  /* a few pixels as border */
	TW_BORDER_REG   /* a window like border, with title on it? */
};

struct tw_border {
	size_t stitle; /* size of title, not used in pixel type */
	size_t sborder; /* the size of top, not used in regular type */

	/* NOTATION: the border should be the same for every output, so stores
	 * the tw_border for every border is really a waste */
};


struct view_list {
	tw_list link;
	tw_handle view;
};
struct view_node {
	tw_list node;
	tw_handle view;
};

/**
 * @brief surface node has a special place to hold wl_surface data structure
 */
struct tw_surface_node {
	tw_list node;
	struct wl_resource *wl_surface;
	tw_handle wlc_view;
};


/**
 *
 * @brief the link data structures used for views (maybe also in other dss)
 */
typedef union view_link {
	struct view_node n;
	struct view_list l;
	//and others
} view_link;

enum STATIC_DRAW_STAGES {
	TW_BACKGROUND = 0,
	TW_PANEL = 1,
	TW_PARTION= 2,
	TW_WIDGET = 3,
	TW_LOCK = 4,
	TWST_NSTAGES=5
};

/**
 * The data that binds to a wlc_view
 */
struct tw_view_data {
	// in relayout method, we always assume the geometry is the content
	// geometry, that is, borders are not included
	tw_border_t type; 
	const tw_border *border; //borders should vary from views to views
	size_t scale; // windows can have their own scales, for those which
		      // doesn't support HIDPI, default 1. It varis from monitors to monitors
	tw_size actual; // when the view need to be scaled, we stores the actual size for it
	view_link link;
	

};



/**
 *
 * @brief, window manager struct, one per output
 */
struct tw_monitor {
	tw_handle output; ///unique id
	size_t scale; //scale is for boarders, main-windows, should be either one, two, three
	//physical attributes
	tw_geometry geometry; // the valid area for create windows.
	//and we have a menu, a very powerful one.
	
	size_t nlayouts;
	//this works for one monitor only, if I have multiple monitor,
	//I should be able to stay on different layout at different monitor
	Layout **layouts; /* array of layout pointers */
	size_t lays_recent[2]; /** recent two layouts */
	//init border for the monitor
	struct tw_border border;

	//the position in the global compositor
	tw_point location;
	//basically static_view is 
//	tw_list static_views;
	struct tw_surface_node static_views[TWST_NSTAGES];
};

struct tw_surface_node *is_static_view_for_output(const tw_handle view, tw_handle output);
/*** monitor specific apis ***/
const static inline tw_geometry *
tw_output_get_geometry(wlc_handle output)
{
	return &((struct tw_monitor *)
		 wlc_handle_get_user_data(output))->geometry;
}



///////////////////////Layout Structures/////////////////////////////
/* I may need to implement a double link list, single link list, and a tree */

/**
 * @brief base layout, need to be implemented by specific layout.
 */
class Layout {
protected:
	/* base Layout class, 
	   it provides basic array view data type.
	 */
	tw_handle monitor;//the monitor it belongs to
	//this should be a circle array, since I can but
	size_t nviews;	//all the views for that layout of the monitor
	
	//the array type of views, this array should always be available
	tw_handle *views;

	//double link list for headers
	tw_list header;

	int offset_next;
public:
	//replace getvarr with update_views. Call update_views everytime the
	//view information changes
	bool update_views(void);
	/* every sub-class inherit this method */
	virtual void relayout (tw_handle output) = 0;
	virtual bool createView (tw_handle view) = 0;
	virtual void destroyView(tw_handle view) = 0;
	///return the location of the view, here it use brute force
	virtual int getViewLoc(const tw_handle view);

	/**
	 * @brief get a tw_handle relates to certain @offset of view
	 *
	 * search on the forwards first, if not found, search backwards
	 */
	virtual const tw_handle getViewOffset(const tw_handle view, size_t offset);
	///the subclass need to override if they operate on different data type.
};

/* the pure floating layout, no stacked/titing window exists */
class FloatingLayout : public Layout {
public:
	FloatingLayout(tw_handle output) {
		this->nviews = 0;
		this->views = NULL;
		this->monitor = output;
		tw_list_init(&this->header);
	}
	bool createView(tw_handle view);
	void relayout(tw_handle output);
	void destroyView(tw_handle view);
};

/**
 *
 * @brief the classic DWM like layout method, support row-based and 
 * col-based.
 * 
 */
class MasterLayout : public Layout {
	//there are two layouts) and the other words, it should run under any
	//resolution
	bool col_based;     //default two column method
	size_t nmaster;
	double master_size; // from 0 to 1, default 0.5
	//append the titling windows to the left, floating windows to the end
	size_t nfloating;

	//deprecated methods
//	tw_handle *getvarr(void);
//	void rmvarr(void);

public:
	MasterLayout(tw_handle output) :
		col_based(true), nmaster(2), master_size(0.5), nfloating(0)
	{
		tw_list_init(&this->header);
		this->monitor = output;
		this->nviews = 0; this->views = NULL;
	}
	void relayout(tw_handle output);
	bool createView(tw_handle view);
	void destroyView(tw_handle view);

	int getViewLoc(const tw_handle view);
	const tw_handle getViewOffset(const tw_handle, size_t);
};


#endif /* EOF */
