/* Copyright AT&T Bell Laboratories, 1993 */
#include <stddef.h>

extern void *memcpy(void*, void*, size_t);

void *
memmove(void *to, void *from, register size_t n)
{
	register char *out = to;
	register char *in = from;

	if(n <= 0)	/* works if size_t is signed or not */
		;
	else if(in + n <= out || out + n <= in)
		return(memcpy(to, from, n));	/* hope it's fast*/
	else if(out < in)
		do
			*out++ = *in++;
		while(--n > 0);
	else {
		out += n;
		in += n;
		do
			*--out = *--in;
		while(--n > 0);
	}
	return(to);
}
