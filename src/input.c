#include <stdbool.h>
#include <wlc/wlc.h>
#include <unistd.h>
#include <stdlib.h>
#include "handlers.h"

/** @brief handle keyboard input
 * 
 *  The keyboard input v0:
 *  - Only one state input, without any buffer cmd
 *  - Using map to search command.
 */
