#ifndef	__COH_FAKEDMA_H__
#define	__COH_FAKEDMA_H__

#include <common/ccompat.h>
#include <common/__caddr.h>
#include <common/__paddr.h>

/* prototypes from fakedma.c */

int	pxcopy		__PROTO ((__paddr_t src, __caddr_t dst,
			  int bytecount, int spaceflag));
int	xpcopy		__PROTO ((__caddr_t src, __paddr_t dst,
			  int bytecount, int spaceflag));

#endif	/* ! defined (__COH_FAKEDMA_H__) */
