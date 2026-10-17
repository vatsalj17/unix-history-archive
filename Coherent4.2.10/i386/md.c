/* $Header: /ker/i386/RCS/md.c,v 2.5 93/10/29 00:57:11 nigel Exp Locker: nigel $ */
/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 2.3.37
 *	Copyright (c) 1982, 1983, 1984.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * Coherent 386
 *	IBM PC
 * Machine dependent stuff.
 *
 * $Log:	md.c,v $
 * Revision 2.5  93/10/29  00:57:11  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.4  93/09/02  18:11:23  nigel
 * Minor edits to prepare for DDI/DKI integration
 * 
 * Revision 2.3  93/08/19  03:40:04  nigel
 * Nigel's R83
 */

#define	_DDI_DKI	1

#include <common/_tricks.h>
#include <sys/types.h>
#include <sys/inline.h>
#include <sys/confinfo.h>
#include <stddef.h>

#include <kernel/trace.h>
#include <kernel/reg.h>
#include <kernel/ddi_cpu.h>
#include <kernel/intr.h>

#include <sys/coherent.h>

/* The following must be nonzero to run MWC X11r5 Xfree1.2's window manager. */
extern int	X00_fix;

#if	__USE_PROTO__
__LOCAL__
void spurious_interrupt (int intr, __int_control_t * __NOTUSED (intrp))
#else
__LOCAL__ void
spurious_interrupt (intr, intrp)
int		intr;
__int_control_t * intrp;
#endif
{
}

__int_control_t		__interrupts [16] = {
	__DECLARE_INT (-1, spurious_interrupt, 0),
	__DECLARE_INT (-1, spurious_interrupt, 1),
	__DECLARE_INT (-1, spurious_interrupt, 2),	/* not used */
	__DECLARE_INT (-1, spurious_interrupt, 3),
	__DECLARE_INT (-1, spurious_interrupt, 4),
	__DECLARE_INT (-1, spurious_interrupt, 5),
	__DECLARE_INT (-1, spurious_interrupt, 6),
	__DECLARE_INT (-1, spurious_interrupt, 7),
	__DECLARE_INT (-1, spurious_interrupt, 8),
	__DECLARE_INT (-1, spurious_interrupt, 9),
	__DECLARE_INT (-1, spurious_interrupt, 10),
	__DECLARE_INT (-1, spurious_interrupt, 11),
	__DECLARE_INT (-1, spurious_interrupt, 12),
	__DECLARE_INT (-1, spurious_interrupt, 13),
	__DECLARE_INT (-1, spurious_interrupt, 14),
	__DECLARE_INT (-1, spurious_interrupt, 15)
};


/*
 */

#if	__USE_PROTO__
void __set_interrupt (intr_t * intrp)
#else
void
__set_interrupt (intrp)
intr_t	      *	intrp;
#endif
{
	int		vector;
	short		s;

	ASSERT (intrp != NULL);

	vector = intrp->int_vector;

	ASSERT (vector >= 0 && vector < __ARRAY_LENGTH (__interrupts));
	ASSERT ((intrp->int_mask & (1 << vector)) != 0);

	s = sphi ();

	__interrupts [vector]._int_mask = intrp->int_mask;
	__interrupts [vector]._int_func = (__int_func_t) intrp->int_handler;
	__interrupts [vector]._int_arg = intrp->int_unit;

	if (__interrupts [vector]._int_func != NULL)
		__SET_BASE_MASK (__GET_BASE_MASK () & ~ (1 << vector));
	else {
		__interrupts [vector]._int_func = spurious_interrupt;
		__SET_BASE_MASK (__GET_BASE_MASK () | (1 << vector));
	}

	spl (s);
}


#if	__USE_PROTO__
int setivec (unsigned int level, int (*fun)())
#else
int
setivec (level, fun)
unsigned int	level;
int		(* fun)();
#endif
{
	intr_t		newint;

	if (level < 1 || level > __ARRAY_LENGTH (__interrupts))
		return 0;

	/*
	 * Create a fake entry for on-the-fly interrupts, at IPL 6 just to
	 * leave some room for manuevering.
	 */

	newint.int_vector = level == 2 ? 9 : level;
	newint.int_handler = fun;
	newint.int_unit = level;
	newint.int_mask = _masktab [6] | (1 << level);

	__set_interrupt (& newint);
	return 1;
}

#if	__USE_PROTO__
void clrivec (int level)
#else
void
clrivec (level)
int		level;
#endif
{
	intr_t		newint;

	newint.int_vector = level == 2 ? 9 : level;
	newint.int_handler = NULL;
	newint.int_unit = level;
	newint.int_mask = -1;
	__set_interrupt (& newint);
}


/*
 * Convert an array of filesystem 3 byte
 * numbers to longs. This routine, unlike the old one,
 * is independent of the order of bytes in a long.
 * Bytes have 8 bits, though.
 */

#if	__USE_PROTO__
void l3tol (long * lp, unsigned char * cp, unsigned int nl)
#else
void
l3tol (lp, cp, nl)
long * lp;
unsigned char * cp;
unsigned int nl;
#endif
{
	long l;

	if (nl != 0) {
		do {
			l  = (long) cp [0] << 16;
			l |= (long) cp [1];
			l |= (long) cp [2] << 8;
			cp += 3;
			* lp ++ = l;
		} while (-- nl);
	}
}


/*
 * Convert an array of longs into an array
 * of filesystem 3 byte numbers. This routine, unlike
 * the old one, is independent of the order of bytes in
 * a long. Bytes have 8 bits.
 */

#if	__USE_PROTO__
void ltol3(char * cp, long * lp, unsigned int nl)
#else
void
ltol3(cp, lp, nl)
char * cp;
long * lp;
unsigned int nl;
#endif
{
	long l;

	if (nl != 0) {
		do {
			l = * lp ++;
			cp [0] = l >> 16;
			cp [1] = l;
			cp [2] = l >> 8;
			cp += 3;
		} while (-- nl);
	}
}


/*
 * Given a port number and a bit value, write the bit value into
 * the tss iomap.
 *
 * Bit value of 0 enables user I / O for that port.
 * Bit value of 1 disables user I / O for that port.
 *
 * Return 1 if port number is valid for the bitmap, else 0.
 */

#if	__USE_PROTO__
int kiopriv(unsigned int port, unsigned int bit)
#else
int
kiopriv (port, bit)
unsigned int port, bit;
#endif
{
	extern long tssIoMap;
	extern long tssIoEnd;
	int ret = 0;
	long * ip;
	unsigned int offset = port >> 5;
	int shift = port & 0x1f;
	long mask = 1 << shift;
	long val = (bit & 1) << shift;

	if (offset < (& tssIoEnd - & tssIoMap)) {
		ip = (& tssIoMap) + offset;
		* ip &= ~ mask;		/* clear old bit value */
		* ip |= val;		/* or in desired new bit value */
		ret = 1;
	}
	return ret;
}


/*
 * Given a 32 bit mask and a word offset into the tss io map,
 * bitwise or the mask into the map.
 * Offset of 0 covers ports 0..31, offset of 1 covers ports 32..63, etc.
 * Current valid range for offset is 0..63, covering ports 0..7ff.
 *
 * Return the new map word.
 */

#if	__USE_PROTO__
long iomapOr (long val, unsigned int offset)
#else
long
iomapOr (val, offset)
long val;
unsigned offset;
#endif
{
	extern long tssIoMap;
	extern long tssIoEnd;
	long ret = -1;
	long * ip;

	if (offset < (& tssIoEnd - & tssIoMap)) {
		ip = (& tssIoMap) + offset;
		ret = * ip |= val;
	}
	return ret;
}


/*
 * Given a 32 bit mask and a word offset into the tss io map,
 * bitwise and the mask into the map.
 * Offset of 0 covers ports 0..31, offset of 1 covers ports 32..63, etc.
 * Current valid range for offset is 0..63, covering ports 0..7ff.
 *
 * Return the new map word.
 */

#if	__USE_PROTO__
long iomapAnd (long val, unsigned int offset)
#else
long
iomapAnd (val, offset)
long val;
unsigned int offset;
#endif
{
	extern long tssIoMap;
	extern long tssIoEnd;
	long ret = -1;
	long * ip;

	if (offset < (& tssIoEnd - & tssIoMap)) {
		ip = (& tssIoMap) + offset;
		ret = * ip &= val;
	}
	return ret;
}

/*
 * Enable user i/o to all ports in iomap.
 * This is support for console ioctl KDENABIO, which takes no arguments.
 */
#if __USE_PROTO__
void __kdenabio(void)
#else
void
__kdenabio()
#endif
{
	extern long tssIoMap;
	extern long tssIoEnd;

	long * ip;

	for (ip = & tssIoMap; ip < & tssIoEnd; ip++)
		*ip = 0;
}

/*
 * Disable user i/o to all ports in iomap.
 * This is support for console ioctl KDDISABIO, which takes no arguments.
 */
#if __USE_PROTO__
void __kddisabio(void)
#else
void
__kddisabio()
#endif
{
	extern long tssIoMap;
	extern long tssIoEnd;

	long * ip;

	if(!X00_fix)
		for (ip = & tssIoMap; ip < & tssIoEnd; ip++)
			*ip = 0xffffffff;
}

/*
 * Enable user i/o to a single port in iomap.
 * This is support for console ioctl KDADDIO, which takes as argument
 * an unsigned short port number.
 */
#if __USE_PROTO__
void __kdaddio(unsigned short port)
#else
void
__kdaddio(port)
unsigned short port;
#endif
{
	kiopriv(port, 0);
}

/*
 * Disable user i/o to a single port in iomap.
 * This is support for console ioctl KDDELIO, which takes as argument
 * an unsigned short port number.
 */
#if __USE_PROTO__
void __kddelio(unsigned short port)
#else
void
__kddelio(port)
unsigned short port;
#endif
{
	if(!X00_fix)
		kiopriv(port, 1);
}
