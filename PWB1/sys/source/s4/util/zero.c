static char Sccsid[] "@(#)zero	1.1";
/*
	Zero `cnt' bytes starting at the address `ptr'.
	Return `ptr'.
*/

struct {
	int	*ip;
};

zero(ptr,cnt)
char *ptr;
int cnt;
{
	register char *p, *w;

	if (cnt) {
		if ((p = ptr) & 1) {
			*p++ = 0;
			--cnt;
		}
		w = p + (cnt & (~1));
		while (p < w)
			*(p.ip)++ = 0;
		if (cnt & 1)
			*p = 0;
	}
	return(ptr);
}
