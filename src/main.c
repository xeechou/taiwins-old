#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <wlc/wlc.h>
#include <wlc/geometry.h>
#include <wlc/wlc-render.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <utils.h>
#include "debug.h"
#include "handlers.h"
#include "protocols.h"
#include <wayland-server.h>

//let just put it here
FILE *debug_file = NULL;
struct tw_compositor compositor;
//extern void register_background(void);

static void
wait_children(int signum)
{
	pid_t pid = wait(NULL);
	debug_log("wait_children called\n");
}

static bool
start_interactive_action(wlc_handle view, const struct wlc_point *origin)
{
	if (compositor.action.view)
		return false;
	compositor.action.view = view;
	compositor.action.grab = *origin;
	wlc_view_bring_to_front(view);
	return true;
}

static void
start_interactive_move(wlc_handle view, const struct wlc_point *origin)
{
	start_interactive_action(view, origin);
}

static void
start_interactive_resize(wlc_handle view, uint32_t edges, const struct wlc_point *origin)
{
	const struct wlc_geometry *g;
	if (!(g = wlc_view_get_geometry(view)) || !start_interactive_action(view, origin))
		return;

	const int32_t halfw = g->origin.x + g->size.w / 2;
	const int32_t halfh = g->origin.y + g->size.h / 2;

	if (!(compositor.action.edges = edges)) {
		compositor.action.edges = (origin->x < halfw ? WLC_RESIZE_EDGE_LEFT : (origin->x > halfw ? WLC_RESIZE_EDGE_RIGHT : 0)) |
			(origin->y < halfh ? WLC_RESIZE_EDGE_TOP : (origin->y > halfh ? WLC_RESIZE_EDGE_BOTTOM : 0));
	}

	wlc_view_set_state(view, WLC_BIT_RESIZING, true);
}

static void
stop_interactive_action(void)
{
	if (!compositor.action.view)
		return;

	wlc_view_set_state(compositor.action.view, WLC_BIT_RESIZING, false);
	memset(&compositor.action, 0, sizeof(compositor.action));
}

static wlc_handle
get_topmost(wlc_handle output, size_t offset)
{
	size_t memb;
	const wlc_handle *views = wlc_output_get_views(output, &memb);
	return (memb > 0 ? views[(memb - 1 + offset) % memb] : 0);
}


/*
void relayout(wlc_handle output)
{
	const struct wlc_size *r;
	if (!(r = wlc_output_get_resolution(output)))
		return;
	size_t memb;
	const wlc_handle* views = wlc_output_get_views(output, &memb);
	relayout_float(views, memb);
}
*/


/*
void
relayout(wlc_handle output)
{
	//XXX: the first window cannot be ranged.
	//But, if I set the function to pre-render hook, the problem will solved
	fprintf(debug_file, "relayout\n");
	struct wlc_geometry g;
	g.origin = (struct wlc_point){0,0};
	g.size = *wlc_output_get_resolution(output);
	size_t x, y, w, h;
	x = 0, y = 0;
	w = g.size.w, h = g.size.h;
	double master_size = 0.5;
	bool col_based = false;
	size_t memb;
	size_t nfloating = 0;
	size_t nmaster = 2;//nmaster may be larger than ninstack or even nviews
	const wlc_handle *views = wlc_output_get_views(output, &memb);
	size_t nviews = memb;
	size_t msize = (size_t)(master_size * ((col_based) ? w : h));
	fprintf(debug_file, "msize is %d\n", msize);
	size_t ninstack = nviews - nfloating;
	
	//assert(nfloating <= nviews && nmaster <= ninstack);

	//we need to setup a counter here, since nmaster maybe larger than nviews
	size_t counter = (ninstack > nmaster) ? nmaster : ninstack;
	//the numbers are correct
	fprintf(debug_file, "counter is %d\n", counter);
	fprintf(debug_file, "arrange %d masters\n", counter);
	fprintf(debug_file, "arrange %d miners\n", ninstack-counter);
	fprintf(debug_file, "arrange %d floating\n", nviews-ninstack);
	fflush(debug_file);

	{//arrange master windows
		if (counter == 0)
			return;
		if (col_based) {
			g.size.w = (ninstack > nmaster) ? msize : w;
			relayout_onecol(views, counter, &g);
		} else {
			g.size.h = (ninstack > nmaster) ? msize : h;
			relayout_onerow(views, counter, &g);
		}
	}
	{//arrange minor windows
		if (ninstack - counter == 0)
			return;
		if (col_based) {
			g.origin.x = x+msize;
			relayout_onecol(views + counter, ninstack-counter, &g);
		} else {
			g.origin.y = y+msize;
			relayout_onerow(views + counter, ninstack-counter, &g);
		}//recover the output-geometry
		g.origin.x = 0; g.origin.y = 0;
		g.size.h = h; g.size.w = w;
	}
	//arrange the floating windows
	relayout_float(views + ninstack, nfloating, &g);
	//	       tw_output_get_geometry(output));

	//it is useless here: wlc_output_schedule_render(output);
}
*/


void
resolution_change_hook(wlc_handle output, const struct wlc_size *from, const struct wlc_size *to)
{
	(void)from, (void)to;
	relayout(output);
}
/*
bool
view_created(wlc_handle view)
{
	fprintf(debug_file, "view created\n");
	fflush(debug_file);
	wlc_view_set_mask(view, wlc_output_get_mask(wlc_view_get_output(view)));
	wlc_view_bring_to_front(view);
	wlc_view_focus(view);
	relayout(wlc_view_get_output(view));
	return true;
}

void
view_destroyed(wlc_handle view)
{
	wlc_view_focus(get_topmost(wlc_view_get_output(view), 0));
	relayout(wlc_view_get_output(view));
}
*/
void
view_focus(wlc_handle view, bool focus)
{
	wlc_view_set_state(view, WLC_BIT_ACTIVATED, focus);
}

void
view_request_move(wlc_handle view, const struct wlc_point *origin)
{
	start_interactive_move(view, origin);
}

void
view_request_resize(wlc_handle view, uint32_t edges, const struct wlc_point *origin)
{
	start_interactive_resize(view, edges, origin);
}

bool
keyboard_key(wlc_handle view, uint32_t time, const struct wlc_modifiers *modifiers, uint32_t key, enum wlc_key_state state)
{
	(void)time, (void)key;

	const uint32_t sym = wlc_keyboard_get_keysym_for_key(key, NULL);

	if (state == WLC_KEY_STATE_PRESSED) {
		if (view) {
			if (modifiers->mods & WLC_BIT_MOD_CTRL && sym == XKB_KEY_q) {
				wlc_view_close(view);
				debug_log("release the view\n");
				return true;
			} else if (modifiers->mods & WLC_BIT_MOD_CTRL && sym == XKB_KEY_Down) {
				wlc_view_send_to_back(view);
				wlc_view_focus(get_topmost(wlc_view_get_output(view), 0));
				return true;
			}
		}

		if (modifiers->mods & WLC_BIT_MOD_CTRL && sym == XKB_KEY_Escape) {
			wlc_terminate();
			return true;
		} else if (modifiers->mods & WLC_BIT_MOD_CTRL && sym == XKB_KEY_Return) {
			char *terminal = (getenv("TERMINAL") ? getenv("TERMINAL") : "weston-terminal");
			wlc_exec(terminal, (char *const[]) { terminal, NULL });
			return true;
		} else if (modifiers->mods & WLC_BIT_MOD_CTRL && sym == XKB_KEY_f) {
			char *flower = "weston-flower";
			wlc_exec(flower, (char *const[]) {flower, NULL});
			return true;
		}
	}

	return false;
}


bool
pointer_button(wlc_handle view, uint32_t time, const struct wlc_modifiers *modifiers, uint32_t button, enum wlc_button_state state, const struct wlc_point *position)
{
	(void)button, (void)time, (void)modifiers;

	if (state == WLC_BUTTON_STATE_PRESSED) {
		wlc_view_focus(view);
		if (view) {
			if (modifiers->mods & WLC_BIT_MOD_CTRL && button == BTN_LEFT)
				start_interactive_move(view, position);
			if (modifiers->mods & WLC_BIT_MOD_CTRL && button == BTN_RIGHT)
				start_interactive_resize(view, 0, position);
		}
	} else {
		stop_interactive_action();
	}

	return (compositor.action.view ? true : false);
}

bool
pointer_motion(wlc_handle handle, uint32_t time, const struct wlc_point *position)
{
	(void)handle, (void)time;

	if (compositor.action.view) {
		const int32_t dx = position->x - compositor.action.grab.x;
		const int32_t dy = position->y - compositor.action.grab.y;
		struct wlc_geometry g = *wlc_view_get_geometry(compositor.action.view);

		if (compositor.action.edges) {
			const struct wlc_size min = { 80, 40 };

			struct wlc_geometry n = g;
			if (compositor.action.edges & WLC_RESIZE_EDGE_LEFT) {
				n.size.w -= dx;
				n.origin.x += dx;
			} else if (compositor.action.edges & WLC_RESIZE_EDGE_RIGHT) {
				n.size.w += dx;
			}

			if (compositor.action.edges & WLC_RESIZE_EDGE_TOP) {
				n.size.h -= dy;
				n.origin.y += dy;
			} else if (compositor.action.edges & WLC_RESIZE_EDGE_BOTTOM) {
				n.size.h += dy;
			}

			if (n.size.w >= min.w) {
				g.origin.x = n.origin.x;
				g.size.w = n.size.w;
			}

			if (n.size.h >= min.h) {
				g.origin.y = n.origin.y;
				g.size.h = n.size.h;
			}

			wlc_view_set_geometry(compositor.action.view, compositor.action.edges, &g);
		} else {
			g.origin.x += dx;
			g.origin.y += dy;
			wlc_view_set_geometry(compositor.action.view, 0, &g);
		}

		compositor.action.grab = *position;
	}

	// In order to give the compositor control of the pointer placement it needs
	// to be explicitly set after receiving the motion event:
	wlc_pointer_set_position(position);
	return (compositor.action.view ? true : false);
}

void view_request_geometry(wlc_handle view, const struct wlc_geometry* g)
{
	(void)view, (void)g;
}



int
main(int argc, char *argv[])
{
	setup_wlc_logger("/home/developer/tw-log");
	debug_file = fopen("/tmp/tw-log", "w+");
	wlc_log_set_handler(wlc_logger);

	//output callbacks
	wlc_set_output_created_cb(output_created);//this get called everytime I switched between sessions
	wlc_set_output_destroyed_cb(output_destroyed);
	wlc_set_compositor_ready_cb(compositor_ready_hook);
	//output callbacks
	wlc_set_view_request_geometry_cb(view_request_geometry);
	wlc_set_view_request_move_cb(view_request_move);
	wlc_set_view_request_resize_cb(view_request_resize);
	wlc_set_view_created_cb(view_created);
	wlc_set_view_destroyed_cb(view_destroyed);
	wlc_set_view_focus_cb(view_focus);
	wlc_set_keyboard_key_cb(keyboard_key);
	wlc_set_pointer_button_cb(pointer_button);
	wlc_set_pointer_motion_cb(pointer_motion);
	wlc_set_output_render_pre_cb(output_pre_render);
	//wlc_set_output_render_pre_cb(relayout);
	//fprintf(stdout, "this line should be printed though\n");
	if (!wlc_init())
		return EXIT_FAILURE;
	tw_globals_registre();
	//we need to have a background global...
	//register_background();
//	wl_global_create(wlc_get_wl_display(), &taiwins_shell_interface, 1, NULL, bind_dummy);
	if (signal(SIGCHLD, wait_children) == SIG_ERR)
	    return -1;

	wlc_run();
	return EXIT_SUCCESS;
}
