#ifndef TW_INPUT_H
#define TW_INPUT_H

#include <assert.h>
#include <stdint.h>
#include <wlc/wlc.h>


/** 
 * @brief key handler struct, this struct should be unique for every differernt
 * key input situation
 */
struct keyinput {
	struct wlc_modifiers modifiers;
	uint32_t keysym;
	uint64_t context;
};

static inline bool
operator==(struct keyinput& left, struct keyinput& right)
{
	return (left.context == right.context) &&
		(left.keysym == right.keysym) &&
		(left.modifiers.leds == right.modifiers.leds) &&
		(left.modifiers.mods == right.modifiers.mods);
}

static inline bool
operator!=(struct keyinput& left, struct keyinput& right)
{
	return (left.context != right.context) ||
		(left.keysym != right.keysym) ||
		(left.modifiers.leds != right.modifiers.leds) ||
		(left.modifiers.mods != right.modifiers.mods);
}

enum key_context_type {
	CONTEXT_IMMEDIATE,
	CONTEXT_LASTING,
};
	
/**
 * @brief key handle context stack
 *
 * One important property is: if we have no context, code should be zero!!!  
 *
 * I use this struct because I want to pop the context everytime I try to handle
 * key, and it should support encoding and decoding operation. So this stack
 * should have length?  if we specific length one, it means all contexts are
 * exclusive, if we pop every time, it means the context will not last.  is
 * there any possible
 */
struct key_context {
	key_context_type type;
	uint64_t code;
	//the method replaces insert function
	struct key_context operator=(struct key_context& rhs) {
		type = rhs.type;
		code = rhs.code;
		return *this;
	};
};


uint64_t
context_code(struct key_context *context)
{
	uint64_t code;
	code = context->code;//if we have no code, it should be zero, but we
			     //cannot gurrante that?
	if (context->type == CONTEXT_IMMEDIATE)
		context->code = 0;
	return code;
}

/**
 * @brief quit an lasting context
 */
void
context_quit(struct key_context *context)
{
	assert(context->type == CONTEXT_LASTING);
	context->code = 0;
}


#endif /* TW_INPUT_H */
