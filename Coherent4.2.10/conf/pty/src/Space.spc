/*
 * Configurable information for "pty" (pseudoterminal) device driver.
 */

#define __KERNEL__	 1

#include "conf.h"

/* Maximum number of pty devices allowed at a time. */
int NUPTY = NUPTY_SPEC;
