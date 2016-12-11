#ifndef __DEBUG_UTILS_H__
#define __DEBUG_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>	
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>	
#include <wlc/wlc.h>
/* debug */

/* this is for debug use, we will mute all this at release */
extern FILE *debug_file;
static inline int debug_log(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(debug_file, format, args);
	va_end(args);
	fflush(debug_file);
	return 0;
}	
	
#ifdef __cplusplus
}
#endif

#endif
