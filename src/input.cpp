#include <stdbool.h>
#include <wlc/wlc.h>
#include <unistd.h>
#include <stdlib.h>
#include "handlers.h"
#include <map>


/* how to use this:
   first: set up all the fields in data
   then:  get code from keymap_code.code
   and vice-versa.
*/
typedef union keymap_code {
	struct {
		uint64_t udefs:8;	/** user defined data, here we use it for compositor mode(resize, locked, etc) */
		uint64_t leds:8; 	/** compositor mode */
		uint64_t mods:16;   	/** modifiers */
		uint64_t keysym:32; 	/** key symbols */
	} data;
	uint64_t code;
} keymap_code;

typedef uint8_t keymap_udefs_t;
typedef uint8_t keymap_leds_t;
typedef uint16_t keymap_mod_t;
typedef uint32_t keymap_keysym_t;



static uint64_t
keysym_encode(const struct wlc_modifiers *modifiers, const uint32_t key, keymap_udefs_t user_data)
{
	keymap_code code;
	code.data.mods = modifiers->mods;
	code.data.keysym = key;
	code.data.udefs = user_data;
	return code.code;
}

static keymap_udefs_t
keysym_decode(uint64_t code, struct wlc_modifiers *modifiers, uint32_t *key)
{
	keymap_code kc;
	kc.code = code;
	modifiers->mods = kc.data.mods;
	modifiers->leds = kc.data.leds;
	*key = kc.data.keysym;
	return kc.data.udefs;
}


enum compositor_state
{
	TW_VIEW_FOCUSED = 1<<0,
	TW_LOCKED = 1<<1,
	TW_OTHER = 1<<2
};
std::map<uint64_t, void*> key_reaction_table;

typedef union  {
	int32_t i;
	uint32_t ui;
	void *p;
	double d; 
} tw_parameter;

/** @brief handle keyboard input
 * 
 *  The keyboard input v0:
 *  - Only one state input, without any buffer cmd
 *  - Using map to search command.
 *  - so 
 * return true to prevent sending events to clients: thats how it works
 */
bool
keyboard_key(wlc_handle view, uint32_t time, const struct wlc_modifiers *modifiers, uint32_t key, enum wlc_key_state state)
{
	(void)time;
	const uint32_t sym = wlc_keyboard_get_keysym_for_key(key, NULL);

	/* get compositor mode */
	keymap_udefs_t cmode = 0;
	if (view)
		cmode = cmode | TW_VIEW_FOCUSED;
	uint64_t keysym_code = keysym_encode(modifiers, sym, cmode);
	/* uncomment when we have a compositor, or maybe changed to cmode = compositor.mode;
	if (compositor.locked)
		cmode = cmode | TW_LOCKED;
	*/
	tw_parameter args[4]; //no more than 
	if (state == WLC_KEY_STATE_PRESSED) {
		function_type *reaction = NULL;
		if ((reaction = (funcion_type)key_reaction_table[keysym_code])) {
			
			reaction(args);
		}
		//all the operations is down here, wow? is this possible?
		
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
