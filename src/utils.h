#ifndef __UTILS_H__
#define __UTILS_H__

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

#define tmp_dir() getenv("XDG_RUNTIME_DIR")

/** create a file that only exists in memory */
int utils_create_anonymous_file(off_t size);
/** create a temp file that has variable name */
int utils_create_temp_file(char *fname);

/** time */
void utils_time(char *time_string);

/** setup logger function, call it before logger */	
int logger_setup(const char *fname);
/** the logger function */	
void logger(enum wlc_log_type type, const char *str);



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
