#ifndef WM_H
#define WM_H
/* data types for window manager, provides abstructions for windows and there
 * arrangements:
 * 
 * + monitor: physical monitors.
 *
 * + workspace: stacks on monitors, multiple monitors are allowed to work on
 * single workspace.
 *
 * + clients: windows we draw
 *
 */

#include <stdio.h>
#include <stdlib.h>


#define NLEVEL		16
#define FACTOR		( 1.0 / NLEVEL )
#define MAXMONS		16

struct wm_workspace;
struct wm_monitor;
struct wm_layout;
struct wm_client;
struct wm_barwin;	/* little bar one the top? */
struct wm_clients_man;
struct wm_global;
typedef struct wm_workspace wm_workspace;
typedef struct wm_monitor wm_monitor;
typedef struct wm_layout wm_layout;
typedef struct wm_client wm_client;
typedef struct wm_barwin wm_barwin;
typedef struct wm_clients_man wm_clients_man;
typedef struct wm_global wm_global;

/************* wrapper of wayland structure *************/

#include <wayland-util.h>		/* for u_list, wl_array */
typedef struct wl_list u_list;

/* aliasing function */
#define u_list_init(list)		wl_list_init(list)
#define u_list_insert(list, elem)	wl_list_insert(list, elem)
#define u_list_remove(elem)		wl_list_remove(elem)
#define u_list_length(list)		wl_list_length(list)
#define u_list_empty(list)		wl_list_empty(list)
#define u_list_insert_list(list, other)	wl_list_insert_list(list, other)

#define u_list_for_each(pos, head, member) \
	wl_list_for_each(pos, head, member)

#define u_list_for_each_safe(pos, tmp, head, member) \
	wl_list_for_each_safe(pos, tmp, head, member)

#define u_list_for_each_reverse(pos, head, member) \
	wl_list_for_each_reverse(pos, head, member)

#define u_list_for_each_reverse_safe(pos, tmp, head, member) \
	wl_list_for_each_reverse_safe(pos, tmp, head, member)

/************* wrapper of wayland structure *************/


typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} wm_arg;


static int n_monitor;

/* to make things easier, we use number to index workspaces, monitors, but not
 * clients
 */

struct wm_workspace {
	int id;
	/********** clients management ***********/
	int n_mon;			/* n monitor this workspace is using */
	wm_clients_man *managers;	/* setup for all monitors */
	/********** clients management ***********/
};

struct wm_clients_man {
	wm_monitor *monitor;
	wm_layout  *mode;

	int 	n_each[NLEVEL];		/* n windows each level */
	float  	s_each[NLEVEL];		/* width/height of each level, added up to 1 */
	u_list *clients;
	u_list *float_clients;

	wm_client *selc;		/* current selected client */
};

struct wm_layout {
	int (*attach) (wm_workspace *);
	int (*detach) (wm_workspace *);
	int (*resize) (wm_workspace *);
};


/* monitor help output width and height of a window */
struct wm_monitor {
	int id;
	unsigned int width, height;	/* the actual width and height of monitor */

	unsigned int draw_w, draw_h;	/* the actual space we can draw clients */

	wm_monitor *up;
	wm_monitor *down;
	wm_monitor *left;
	wm_monitor *right;

	wm_workspace *recent[2];
};

/* a wm_client can only belongs to one workspace */
struct wm_client {
	u_list node;
	char name[256];
	wm_monitor *attached_to; 	/* also help to output true x,y,x,h */

	float x, y, w, h;		/* x, y is important to identify the level of clients */
	float old_x, old_y, old_w, old_h;
};

struct wm_global {
	int n_mons;
	wm_monitor *mons;		//index use mons[i]

	int n_spaces;
	wm_workspace *spaces;		//this can be allocate statically

	wm_monitor *selmon;
};
extern wm_global globals;


//use this expression or you can define container of yourself
#define client_of(ptr)  wl_container_of(ptr, (wm_client *)0, node)

#endif
