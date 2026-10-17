/*	@(#)subr_xxx.c 1.1 85/05/30 SMI; from UCB 4.22 83/05/27	*/

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/conf.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/proc.h"
#include "../h/vnode.h"
#include "../h/vm.h"
#include "../h/cmap.h"
#include "../h/uio.h"

/*
 * Routine placed in illegal entries in the bdevsw and cdevsw tables.
 */
nodev()
{

	return (ENODEV);
}

/*
 * Null routine; placed in insignificant entries
 * in the bdevsw and cdevsw tables.
 */
nulldev()
{

	return (0);
}

imin(a, b)
{

	return (a < b ? a : b);
}

imax(a, b)
{

	return (a > b ? a : b);
}

unsigned
min(a, b)
	u_int a, b;
{

	return (a < b ? a : b);
}

unsigned
max(a, b)
	u_int a, b;
{

	return (a > b ? a : b);
}

extern	cabase, calimit;
extern	struct pte camap[];

caddr_t	cacur = (caddr_t)&cabase;
caddr_t	camax = (caddr_t)&cabase;
int	cax = 0;
/*
 * This is a kernel-mode storage allocator.
 * It is very primitive, currently, in that
 * there is no way to give space back.
 * It serves, for the time being, the needs of
 * auto-configuration code and the like which
 * need to allocate some stuff at boot time.
 */
caddr_t
calloc(size)
	int size;
{
	register caddr_t res;
	register int i;

	if (cacur+size >= (caddr_t)&calimit)
		panic("calloc");
	while (cacur+size > camax) {
		(void) vmemall(&camap[cax], CLSIZE, &proc[0], CSYS);
		vmaccess(&camap[cax], camax, CLSIZE);
		for (i = 0; i < CLSIZE; i++)
			clearseg(camap[cax++].pg_pfnum);
		camax += NBPG * CLSIZE;
	}
	res = cacur;
	cacur += size;
	return (res);
}

#ifndef vax
ffs(mask)
	register long mask;
{
	register int i;

	if (mask == 0)
		return(0);
	for(i = 1; i < NSIG; i++) {
		if (mask & 1)
			return (i);
		mask >>= 1;
	}
	return (0);
}

bcmp(s1, s2, len)
	register char *s1, *s2;
	register int len;
{

	while (len--)
		if (*s1++ != *s2++)
			return (1);
	return (0);
}

skpc(c, len, cp)
	register char c;
	register unsigned short len;
	register char *cp;
{
	if (len == 0)
		return (0);
	while (*cp++ == c && --len)
		;
	return (len);
}
#endif

/*
 * Returns the number of
 * non-NULL bytes in string argument.
 */
strlen(s)
	register char *s;
{
	register int n;

	n = 0;
	while (*s++)
		n++;
	return (n);
}

/*
 * Compare strings:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */
strcmp(s1, s2)
	register char *s1, *s2;
{

	while (*s1 == *s2++)
		if (*s1++=='\0')
			return (0);
	return (*s1 - *--s2);
}

strncmp(s1, s2, n)
        register char *s1, *s2;
        register int n;
{

        while (--n >= 0 && *s1 == *s2++)
                if (*s1++ == '\0')
                        return (0);
        return (n<0 ? 0 : *s1 - *--s2);
}

/*
 * Copy string s2 to s1.  s1 must be large enough.
 * return ptr to null in s1
 */
char *
strcpy(s1, s2)
	register char *s1, *s2;
{

	while (*s1++ = *s2++)
		;
	return (s1 - 1);
}

/*
 * Copy s2 to s1, truncating to copy n bytes
 * return ptr to null in s1 or s1 + n
 */
char *
strncpy(s1, s2, n)
	register char *s1, *s2;
{
	register i;

	for (i = 0; i < n; i++) {
		if ((*s1++ = *s2++) == '\0') {
			return (s1 - 1);
		}
	}
	return (s1);
}
