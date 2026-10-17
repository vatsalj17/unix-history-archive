/* @(#)setdomainname.c 1.1 85/05/30 SMI */

#include "SYS.h"

SYSCALL(setdomainname)
	RET		/* setdomainname(name, len) */
