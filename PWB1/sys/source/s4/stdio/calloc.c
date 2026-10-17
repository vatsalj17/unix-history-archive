#include "stdio.h"
/*	calloc - allocate and clear memory block
*/
#define BYTESPERWD (sizeof(int)/sizeof(char))

int *malloc();

int *calloc(num, size)
unsigned num;
{
	register *p, *q;
	register m;
	num =* size;
	if((p=malloc(num)) != NULL) {
		q = p;
		m = (num+BYTESPERWD-1)/BYTESPERWD;
		while(--m>=0)
			*q++ = 0;
	}
	return(p);
}

cfree(p)
int *p;
{
	free(p);
}
