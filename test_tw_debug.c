#include <stdio.h>

#include "tw_utils.h"

int main(int argc, char **argv)
{
	int fd = 0;
	init_debug();

	while(1) {
		TW_DEBUG_INFO("123\n");
		TW_DEBUG_WARNING("456\n");
		TW_DEBUG_ERROR("789\n");
		sleep(2);
	}
	return 0;
}
