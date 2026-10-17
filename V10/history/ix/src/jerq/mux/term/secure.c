#include <jerq.h>
#include <layer.h>
#include <jerqproc.h>
#include <sys/label.h>	/* for LABSIZ */
/* Multilevel security code.  Entirely concerned with
   labeling the snarf buffer.
*/
/* plabtab is a parallel array to proctab; logically part of
   struct Proc, but nobody else needs to know it. 
*/
static char (*plabtab)[LABSIZ];	/* process labels */
static char snarflab[LABSIZ];	/* snarf buffer */
int trusted = 1;
extern char pextab[];
setproclab(i,s)
char *s;
{
	register char *t = plabtab[i];
	for(i=LABSIZ; --i>=0; )
		*t++ = s? *s++: 0;
	return LABSIZ;
}
labinit(nproc)
{
	plabtab = (char(*)[LABSIZ])alloc(nproc*LABSIZ);
}
clearpex(i)
{
	pextab[i] = 0;
}
setbuflab()
{
	register char *s = plabtab[P-proctab];
	register char *t = snarflab;
	register i = LABSIZ;
	while(--i>=0)
		*t++ = *s++;
}
labchk()	/* is it safe to paste into current proc? */
{
	register char *t = plabtab[P-proctab];
	register char *s = snarflab;
	register i = LABSIZ;
	if(trusted)
		while(--i >= 0)
			if(*s++ & ~*t++)
				return 0;
	return 1;
}
extern int NPROC;
jpex(chan,state)
{
	int i;
	if(chan==0) {
		trusted=0;
		for(i=2; i<NPROC; i++)
			clearpex(i);
	}
	else {
		pextab[chan]=state;
		setborder(&proctab[chan]);
		setrun(&proctab[chan]);
	}
}
