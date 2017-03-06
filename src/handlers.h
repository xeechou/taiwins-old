#ifndef TAIWINS_HANDLERS_H
#define TAIWINS_HANDLERS_H

/**
 * @file handlers.h
 * @brief this file declares the function that implemented by other files
 * 
 * every cpp source files include this header!!!
 */


#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#include <wlc/wlc.h>
#include <wlc/geometry.h>
#include <types.h>


/* TODO change this struct later **/
struct tw_compositor {
	struct {
		wlc_handle view;
		struct wlc_point grab;
		uint32_t edges;
	} action;
};

extern struct tw_compositor compositor;

#define utils_max(a,b) ({				\
			__typeof__ (a) _a = (a);	\
			__typeof__ (b) _b = (b); 	\
			return (_a > _b ? _a : _b); })

static inline uint32_t maxu32(uint32_t a, uint32_t b)
{
	return (a > b) ? a : b;
}
static inline uint32_t min32(uint32_t a, uint32_t b)
{
	return (a < b) ? a : b;
}

EXTERNC void compositor_ready_hook(void);

/** hook for resolution change, update layout */
EXTERNC void resolution_change_hook(wlc_handle output, const struct wlc_size *from, const struct wlc_size *to);
	
/** hook for creating new view(window) */
EXTERNC bool view_created(wlc_handle view);

/** hook called when view(window) destroied */
EXTERNC void view_destroyed(wlc_handle view);

/** hook called when view is focused */
EXTERNC void view_focus(wlc_handle view, bool focus);

EXTERNC void relayout(wlc_handle output);

EXTERNC bool keyboard_key(wlc_handle view, uint32_t time, const struct wlc_modifiers *modifiers, uint32_t key, enum wlc_key_state state);

EXTERNC bool pointer_button(wlc_handle view, uint32_t time, const struct wlc_modifiers *modifiers,
		    uint32_t button, enum wlc_button_state state, const struct wlc_point *position);

EXTERNC bool pointer_motion(wlc_handle handle, uint32_t time, const struct wlc_point *position);


/***** output callbacks ****/
/* output create hook */
EXTERNC bool output_created(wlc_handle output);
/* output destroy hook */
EXTERNC void output_destroyed(wlc_handle output);

EXTERNC void output_pre_render(wlc_handle output);

#endif
