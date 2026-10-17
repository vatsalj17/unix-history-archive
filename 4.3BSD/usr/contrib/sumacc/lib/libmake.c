/*	libmake.c	1.0	05/12/84	*/

/*
 * Assemble and archive mac library stubs.
 *
 * This combines the functions of "as" and "ar".  We first tried
 * doing this the "easy" way using as and ar;  it took about 40 minutes
 * real-time to assemble and archive 520+ stubs.  Not to mention the
 * hardships of a UNIX directory containing 1040+ files.
 *
 * Libmake is typically run as:
 * "cat libqd.sl libos.sl ... | sed -f libmac.sed | libmake OUTFILENAME"
 *
 * Standard input looks like:
 * SubName:
 * 	movl	#8,sp@-
 * 	movl	#-43504,sp@-
 *
 * As output, libmake generates an archive of b.out's such as:
 *	.globl	SubName,_mactrap
 * SubName:
 *	jbsr	_mactrap
 *	.long	8		| the arg flags
 *	.long	-43504		| the trap or stub address
 *
 * TODO:
 * Fix ld68 and ranlib(68?) so they can randomly load stuff, rather than
 * having to linearly scan the archive.
 */

/*
 * Copyright (C) 1984, Stanford Univ. SUMEX project.
 * May be used but not sold without permission.
 */

/*
 * history
 * 05/12/84	Croft	Created.
 */

#include <stdio.h>
#include "b.out.h"
#include <ar.h>

struct ao {			/* archive output */
	struct ar_hdr ah;	/* ar.h header */
	struct bhdr bh;		/* b.out.h header */
	short	j;		/* jbsr _mactrap */
	short	h3;
	short	l3;
	short	h1;		/* arg flags */
	short	l1;
	short	h2;		/* trap or stub address */
	short	l2;
	short	unused;		/* hmm, loader seems to need long alignment */
	char	d[256];		/* symbol table and text reloc */
} ao;

int	btsize = 16;		/* 16 bytes in text seg */

struct sym bs;

struct breloc {			/* b.out reloc, like "reloc" but no bit fields*/
	short	rtype;
	short	rsymbol;
	long	rpos;
};

struct breloc brtrap = {
	RLONG<<2 | REXT, 0, 2	/* relocate _mactrap, sym 0, offset 2 */
};
struct breloc brext = {
	RLONG<<2 | REXT, 2, 10	/* relocate stubaddr, sym 2, offset 10 */
};

char	*foutname;
FILE	*fout;
char	*aop;
char	label[64];
char	aflags[32];
char	trap[32];

unsigned long htonl();
unsigned short htons();


main(argc,argv)
	char **argv;
{
	register char *cp;

	foutname = argv[1];
	unlink(foutname);
	if ((fout = fopen(foutname, "w")) <= 0)
		abort("can't open output file");
	fprintf(fout, ARMAG);
	sets(ao.ah.ar_date, sizeof ao.ah.ar_date, "d", time(0));
	sets(ao.ah.ar_uid, sizeof ao.ah.ar_uid, "d", getuid());
	sets(ao.ah.ar_gid, sizeof ao.ah.ar_gid, "d", getgid());
	sets(ao.ah.ar_mode, sizeof ao.ah.ar_mode, "o", 0664);
	sets(ao.ah.ar_fmag, sizeof ao.ah.ar_fmag, "s", ARFMAG);
	
	ao.bh.fmagic = FMAGIC;
	ao.bh.tsize = btsize;

	ao.j = htons(0x4eb9);	/* jbsr */

	/*
	 * for each file
	 */
	for (;;) {	
		if (scanf("%[^:]:\n\tmovl\t#%[^,],sp@-\n\tmovl\t#%[^,],sp@-\n",
		    label, aflags, trap) != 3) {
			if (!feof(stdin)) {
				printf("\t%s/%s/%s\n", label, aflags, trap);
				abort("bad input conversion");
			}
			fclose(fout);
			exit(0);
		}
		makebout();
	}
}


/*
 * Make a single b.out+archive header.
 */
makebout()
{
	int s;

	aop = ao.d;
	makesym(EXTERN|UNDEF, "_mactrap");
	makesym(EXTERN|TEXT, label);
	*((long *)&ao.h1) = htonl(atoi(aflags));
	if (trap[0] == '-') {	/* if trap number */
		*((long *)&ao.h2) = htonl(atoi(trap));
		ao.bh.ssize = aop - ao.d;
		ao.bh.rtsize = sizeof brtrap;
		aoappend((char *)&brtrap, sizeof brtrap);
	} else {		/* symbol stub */
		*((long *)&ao.h2) = 0;
		makesym(EXTERN|UNDEF, trap);
		ao.bh.ssize = aop - ao.d;
		ao.bh.rtsize = (sizeof brtrap + sizeof brext);
		aoappend((char *)&brtrap, sizeof brtrap);
		aoappend((char *)&brext, sizeof brext);
	}
	sets(ao.ah.ar_name, sizeof ao.ah.ar_name, "s", label);
	s = sizeof ao.bh + ao.bh.tsize + ao.bh.ssize + ao.bh.rtsize;
	sets(ao.ah.ar_size, sizeof ao.ah.ar_size, "d", s);
	fwrite((char *)&ao, s + sizeof ao.ah, 1, fout);
	if (s&1)
		putc('\n',fout);
}


/*
 * Set string in archive header.
 */
sets(cp,l,f,v)
	char *cp,*f,*v;
{
	char lin[64];
	char form[16];

	sprintf(form, "%%-%d%s", l, f);
	sprintf(lin, form, v);
	strncpy(cp, lin, l);
}


/*
 * Make a symbol table entry.
 */
makesym(f,s)
	char *s;
{
	bs.stype = f;
	bs.slength = strlen(s);
	aoappend((char *)&bs, sizeof bs);
	aoappend(s, strlen(s)+1);
}


/*
 * Append to ao.
 */
aoappend(cp,n)
	char *cp;
{
	bcopy(cp, aop, n);
	aop += n;
}


/*
 * Abort with message.
 */
abort(s,a,b)
	char *s;
{
	fprintf(stderr, "libmake: ");
	fprintf(stderr, s, a, b);
	fprintf(stderr, "\n");
	exit(1);
}


/*
 * Copy bytes.
 */
bcopy(a, b, n)
	register n;
	register char *a, *b;
{
	if (n <= 0)
		return;
	do { *b++ = *a++; } while (--n);
}


#define nohtonl 
#ifdef nohtonl	/* if not in library */
/*
 * "Host" to "net" byte order swappers.
 */
unsigned short htons(a)
	unsigned short a;
{
	unsigned short result;
	register char *sp = (char *)&a;
	register char *dp = (char *)&result;

	dp[1] = *sp++;
	dp[0] = *sp;
	return (result);
}


unsigned long htonl(a)
	unsigned long a;
{
	unsigned long result;
	register char *sp = (char *)&a;
	register char *dp = (char *)&result;

	dp[3] = *sp++;
	dp[2] = *sp++;
	dp[1] = *sp++;
	dp[0] = *sp;
	return (result);
}
#endif
