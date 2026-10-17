#ifndef	__COH_PROC_H__
#define	__COH_PROC_H__

#include <common/ccompat.h>
#include <sys/sched.h>

/* prototypes from proc.c */
__sleep_t	x_sleep	__PROTO ((char * event, int schedPri,
			  int sleepPri, char * reason));

void		wakeup	__PROTO ((char * event));

#endif	/* ! defined (__COH_PROC_H__) */
