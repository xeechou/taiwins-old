#ifndef TW_LAYOUT_H
#define TW_LAYOUT_H
/**
 *
 * @file layout.h
 * @brief the fine defines different types of layout strategy, need to be include by output struct
 */



#include <types.h>
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
	tw_handle *views; // the view for the given output
	
public:
	/* every sub-class inherit this method */
	virtual void relayout (tw_handle output);
	virtual void createView (tw_handle view);
};


/* the pure floating layout, no stacked/titing window exists */
class Floating : Layout {
	/* the link list seems to be the most promissing data type for floating views */
public:
	void createView(tw_handle view);
	void relayout(tw_handle output);
};


/**
 *
 * @brief the classic DWM like layout method, support row-based and 
 * col-based.
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






#endif /* EOF */
