#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <wlc/wlc.h>
#include <wlc/geometry.h>
#include <wlc/wlc-render.h>
#include <linux/input.h>
#include <utils.h>
#include "handlers.h"

static FILE *debug_file = NULL;
struct tw_compositor compositor;
//extern void register_background(void);

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

static void
relayout_float(const wlc_handle *views, const size_t nviews)
{
	//this method has to be called in safe env.
	if(!views)
		return;
	wlc_handle output = wlc_view_get_output(*views);
	const struct wlc_size *r;
	if (!(r = wlc_output_get_resolution(output)))
		return;
	//this method is not safe, how do you ensure you will not access
	//unallocated memory? c++ vector?
	for (size_t i = 0; i < nviews; i++) {
		//the only thing we are doing here is ensure the the window
		//does not go out or border, but is may not be good, since
		//windows can have border
		struct wlc_geometry g = *wlc_view_get_geometry(views[i]);
		//If I add this line, it will come with wired effect, 
		g.size.h = (g.size.h > 0) ? g.size.h : 100;
		g.size.w = (g.size.w > 0) ? g.size.w : 100;
		
		int32_t view_botton = g.origin.y+g.size.h;
		int32_t view_right = g.origin.x+g.size.w;
		
		g.size.h = MIN(view_botton, r->h) - g.origin.y;
		g.size.w = MIN(view_right, r->w) - g.origin.x;
		
		wlc_view_set_geometry(views[i], 0, &g);
	}
}
void relayout(wlc_handle output)
{
	const struct wlc_size *r;
	if (!(r = wlc_output_get_resolution(output)))
		return;
	size_t memb;
	const wlc_handle* views = wlc_output_get_views(output, &memb);
	relayout_float(views, memb);
}

void
resolution_change_hook(wlc_handle output, const struct wlc_size *from, const struct wlc_size *to)
{
	(void)from, (void)to;
	relayout(output);
}

bool
view_created(wlc_handle view)
{
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



int
main(int argc, char *argv[])
{
	logger_setup("stderr");
	debug_file = fopen("/tmp/tw-log", "w+");
	wlc_log_set_handler(logger);

	wlc_set_view_created_cb(view_created);
	wlc_set_view_destroyed_cb(view_destroyed);
	wlc_set_view_focus_cb(view_focus);
	wlc_set_view_request_move_cb(view_request_move);
	wlc_set_view_request_resize_cb(view_request_resize);
	wlc_set_keyboard_key_cb(keyboard_key);
	wlc_set_pointer_button_cb(pointer_button);
	wlc_set_pointer_motion_cb(pointer_motion);

	if (!wlc_init())
		return EXIT_FAILURE;
	//we need to have a background global...
	//register_background();

	wlc_run();
	return EXIT_SUCCESS;
}
