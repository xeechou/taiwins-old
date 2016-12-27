#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
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
int setup_wlc_logger(const char *fname);
/** the wlc_log function */
void wlc_logger(enum wlc_log_type, const char *str);
extern FILE *logger_file;
/**
 * @brief logging ability,
 */
static inline void
logger(enum wlc_log_type type, const char *str, ...)
{
	va_list args;

//	fprintf(logger_file, "%s: ", logger_type(type));
	va_start(args, str);
	vfprintf(logger_file, str, args);
	fprintf(logger_file, "\n");
	va_end(args);
}
	
#ifdef __cplusplus
}
#endif

#endif
