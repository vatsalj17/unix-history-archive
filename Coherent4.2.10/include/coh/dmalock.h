#ifndef	__COH_DMALOCK_H__
#define	__COH_DMALOCK_H__

#include <common/ccompat.h>
#include <coh/timeout.h>

/* prototypes from dmalock.c */

int	dmalock		__PROTO ((TIM * dfp, __TIMED_FN_PTR, int arg));
void	dmaunlock	__PROTO ((TIM * dfp));

#endif	/* ! defined (__COH_DMALOCK_H__) */
