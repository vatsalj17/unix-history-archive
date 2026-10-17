/* @(#)getdomainname.c 1.1 85/05/30 SMI */

#include "SYS.h"

SYSCALL(getdomainname)
	RET		/* len = getdomainname(buf, buflen) */
