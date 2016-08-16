#ifndef TW_WM_H
#define TW_WM_H
#include <stdlib.h>

#include <types.h>

/* types used in this place */
class Layout;
struct tw_monitor;

//////////////////////////////////////////////////////////////////////////
/////////////////////////View specific structures/////////////////////////
//////////////////////////////////////////////////////////////////////////


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


struct view_list {
	tw_list link;
	tw_handle view;
};
struct view_node {
	tw_list node;
	tw_handle view;
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


/**
 * The data that binds to a wlc_view
 */
struct tw_view_data {
	// in relayout method, we always assume the geometry is the content
	// geometry, that is, borders are not included
	
	const tw_border *border;
	size_t scale; // windows can have their own scales, for those which
		      // doesn't support HIDPI, default 1.
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
};


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
	/* base class */
	tw_handle monitor;//the monitor it belongs to

	//this should be a circle array, since I can but
	size_t nviews;	//all the views for that layout of the monitor
	tw_handle *views; // the view for the given output,
	
public:
	/* every sub-class inherit this method */
	virtual void relayout (tw_handle output) = 0;
	virtual bool createView (tw_handle view) = 0;
	virtual void destroyView(tw_handle view) = 0;
};




/* the pure floating layout, no stacked/titing window exists */
class FloatingLayout : public Layout {
	/* the link list seems to be the most promissing data type for floating views */
public:
	//void createView(tw_handle view);
	void relayout(tw_handle output);
	//void createView(tw_handle view);
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

	//data
	tw_list *header; //header of windows
	//this is a adapter for create a template
	tw_handle *getvarr(void);
	void rmvarr(void);

public:
	MasterLayout(tw_handle output) :
		col_based(true), nmaster(2), master_size(0.5), nfloating(0) { this->monitor = output; this->header = NULL;
		this->nviews = 0;}
	void relayout(tw_handle output);
	bool createView(tw_handle view);
	void destroyView(tw_handle view);
};
//for master layout, the best struct to store views is double link


#endif /* EOF */
