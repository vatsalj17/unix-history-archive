#define TMPFILE	"/tmp/joveXXXXXX"
	/* Where jove should put the tmp file */

#define BIGTMP	1
	/* BIGTMP allows 512k bytes in tmp file.  Otherwise 256k bytes */

#define DESCRIBE	"/usr/lib/jove/describe.com"

#define FINDCOM	"/usr/lib/jove/findcom"
	/* Where to search for the describe command */

/* #define LSRHS_KLUDGERY		/* Probably ONLY if you are at LS */

#define MENLO_JCL		/* If you have job stopping.  Probably
				   not necessary for most systems with
				   job stopping, but ours is in the middle
				   of changing over ... */

#ifdef vax
#define VMUNIX	1		/* Vax unix */
#endif

#ifndef VMUNIX
typedef	short	disk_line;
#define BUFSIZ	512
#else
typedef	long	disk_line;
#define BUFSIZ	1024
#endif
