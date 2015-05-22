#ifndef __TW_UTILS_H__
#define __TW_UTILS_H__

#include <string.h>
/* debug */
extern int tw_log_fd;

#define TW_MSG_INFO "info"
#define TW_MSG_WARNING "warning"
#define TW_MSG_ERROR "error"

#define TW_DEBUG(level, msg, args...) ({                \
		char time_buf[24] = {0};                \
		char msg_buf[256] = {0};                \
		get_time(time_buf);                     \
		sprintf(msg_buf, "[%s][%s:%d:(%s)][%s]\t" msg,time_buf, __FILE__,__LINE__,__func__, level, ##args);             \
		write_to_file(tw_log_fd, msg_buf, strlen(msg_buf));                      \
		})

#define TW_DEBUG_INFO(msg, args...) TW_DEBUG(TW_MSG_INFO, msg, ##args)
#define TW_DEBUG_WARNING(msg, args...) TW_DEBUG(TW_MSG_WARNING, msg, ##args)
#define TW_DEBUG_ERROR(msg, args...) TW_DEBUG(TW_MSG_ERROR, msg, ##args)

#define DEBUG_FILE "/tmp/tw.log"

int init_debug();

/* time */
void get_time(char *time_string);


/* file ops */
int open_file(char *file_name);
int writen_to_file(int fd, char *msg_to_file, unsigned int msg_length);
int readn_from_file(int fd, char *msg_from_file, unsigned int msg_length);

#endif
