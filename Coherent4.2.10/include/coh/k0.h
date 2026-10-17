#ifndef	__COH_K0_H__
#define	__COH_K0_H__

#include <common/ccompat.h>

/* prototypes from k0.s */

int		read_cmos	__PROTO ((int addr));
void		write_cmos	__PROTO ((int addr, int data));
int		read_t0		__PROTO ((void));
unsigned char	getubd		__PROTO ((caddr_t uaddr));

#endif	/* ! defined (__COH_K0_H__) */
