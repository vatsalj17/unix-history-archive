#ifndef DIDUTIL
#define DIDUTIL
#include <stdio.h>                /* minus the ctype stuff */
#include <ctype.h>
#include <setjmp.h>
#include <sys/types.h>
#include <errno.h>

/* declarations that should have been in the system files */

extern char *strcat ();
extern char *strcpy ();
extern char *sprintf ();

/* */

extern jmp_buf timerest;

/* some common logical values */

#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif
#ifndef YES
#define YES     1
#endif
#ifndef NO
#define NO      0
#endif
#ifndef OK
#define OK      0
#endif
#ifndef DONE
#define DONE    1
#endif
#ifndef NOTOK
#define NOTOK   -1
#endif
#ifndef	MAYBE
#define	MAYBE	-2
#endif

/* some C compilers do not support 'global' statics properly */

#ifndef LOCFUN
#define LOCFUN static             /* function local to module           */
#endif
#ifndef LOCVAR
#define LOCVAR static             /* variable local to module           */
#endif

/* stdio extensions */

#define lowtoup(chr) (islower(chr)?toupper(chr):chr)
#define uptolow(chr) (isupper(chr)?tolower(chr):chr)
#define min(a,b) ((b<a)?b:a)
#define isstr(ptr) ((ptr) != 0 && (ptr)[0] != '\0')
#define isnull(chr) ((chr) == '\0')

#define FOREVER   for (;;)

union pipunion
{
    int     pipcall[2];
    struct pipstruct
    {
	int     prd;
	int     pwrt;
    } pip;
};

typedef union pipunion Pip;

#ifdef v6
long	siz2lon();
#define st_gsize(ino_ptr) ((long)(siz2lon((struct stat *)ino_ptr)))
#else
#define st_gsize(inode) ((long)((struct stat *)inode)->st_size)
#endif

#define gwaitval(val)   ((val) >> 8)
				  /* get exit() value from child        */
				  /* val is the argument to wait()      */
				  /* this macro expects to extract the  */
				  /* high byte from the "returned"      */
				  /* value                              */
#endif DIDUTIL
