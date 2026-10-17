#include <setjmp.h>
#include <stdio.h>

static jmp_buf jb;

setjmp_(x)
 long *x;
{
	setjmp(jb);
	}

onbrk_(signo)
{
	printf("Interrupt!\n");
	fflush(stdout);
	longjmp(jb,1);
	}
