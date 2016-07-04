#ifndef TW_COMPOSITOR_H
#define TW_COMPOSITOR_H

#include <stdlib.h>
#include <stdint.h>
#include <wlc/wlc.h>

struct tw_workspace;

typedef wlc_handle handle_t;
/* 
struct tw_compositor {
	struct {
		wlc_handle view;
		struct wlc_point grab;
		uint32_t edges;
	} action;
};
 */

typedef struct tw_monitor {
	/* the geometry information */
	struct tw_monitor *left;
	struct tw_monitor *right;
	struct tw_monitor *top;
	struct tw_monitor *down;
	handle_t handler;
	unsigned int scale;
} tw_monitor;

/**
 * @brief the tw_compositor class
 * 
 * The compositor global structure, usually we only have one copy allocated at
 * start up, it belongs to part of the constant.
 */
class TWcompositor {
private:
	//the list of workspaces, since it is static.
	unsigned int numws;
	tw_workspace *workspaces;
	unsigned int recent_ws[2]; /** recent workspace */

	/** monitor(or we can call it output) information, the one that contains
	 * cursor is the current monitor */
	tw_monitor *current_mon;  /** every output has a list of views */

	/* the status bar information? */
public:
	TWcompositor();
	~TWcompositor() {free(workspaces);};
	//other methods
};



/** 
 * @brief the workspace representation
 *  
 * What we should have in a workspace data structure?
 * 1) a list of windows(views)
 * 2) rules to relayout windows
 * 3) the floating windows, how many floating windows? The window on top should be the first item in the link
 * 4) the pointer to current monitors(output)
 * 5) scale information?
 * 
 */
typedef struct tw_workspace {

} tw_workspace;


#endif /* TW_COMPOSITOR_H */
