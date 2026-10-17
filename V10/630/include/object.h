/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(object, @(#)object.h	1.1.1.5	(5/12/87));

typedef struct Cache {
	struct Obj *fobj;	/* first object in the cache */
	struct Obj *lobj;	/* last object in the cache */
	long num;		/* number of objects in the cache */
	int cindex;		/* cache index */
	long ftf;		/* for the future */
} Cache;

typedef struct Obj {
	char *name;			/* key name of object */
	int type;			/* type of object*/
	int cindex;			/* index of the cache the object resides */
	long ftf;			/* for the future */
	struct Obj_user *user;		/* list of processes using this object */
	struct Obj *next;		/* links to next object in the cache */

	union {	
		struct Appl *cappl;	/* cached applications */
		struct Font *cfont;	/* cached fonts */
		struct Netw *cnetw;	/* not defined yet */
		struct Msg  *cmsg;	/* not defined yet */
		struct Fln  *cfln;	/* not defined yet */
		struct Usr1 *cusr1;	/* user 1 cache */
		struct Usr2 *cusr2;	/* user 2 cache */
		struct Usr3 *cusr3;	/* user 3 cache */
	} info;
} Obj;

typedef struct Obj_user {
	struct Proc *proc;		/* process requesting the object */
	struct Obj_user *next;		/* next process */
} Obj_user;
	

typedef struct Appl {
	long status;		/* status of the Appl object */

	char *text;		/* executing code */
	char *data;		/* data section */
	char *bss;		/* bss section */
	int argc;		/* number of arguments */
	char **argv;		/* strings of arguments */
	unsigned long stksize;	/* stack size */
	unsigned long progid;	/* programming environment ID (not used) */
	Rectangle rect;		/* default window size of application */

	char *caption;		/* text for this item as appeared in More menu */
	struct Tmenu *next;	/* submenu from this item */
        struct Bitmap *icon;	/* icon for this item */
	void (*update)();	/* update for menu generation */
	void (*exec)();		/* execute this after item is selected */

	long ftf;		/* for the future */
} Appl;
	

/* Type of an object */
#define SHARED		0x1	/* no mutual exclusion in usage */
#define PERMANENT	0x2	/* cannot "uncache" it */

/* Cache index */
#define APPLCACHE		0x0	/* applications (supported) */
#define FONTCACHE		0x1	/* fonts (supported) */
#define NETWCACHE		0x2	/* network drivers (reserved) */
#define MSGCACHE		0x3	/* messages (reserved) */
#define FILECACHE		0x4	/* files and data (reserved) */
#define USER1CACHE		0x5	/* open */
#define USER2CACHE		0x6	/* open */
#define USER3CACHE		0x7	/* open */
#define NCACHE			0x8	/* total: eight caches */		


/* Status of an APPL object */
#define SWEEP		(long)0x1	/* can run in different size windows */
#define CONNECT		(long)0x2	/* runs as connected to a host */
#define SHOWOFF		(long)0x4	/* shows off on the More menu */
#define COMMAND		(long)0x8	/* is a command, not a window application */
#define NOTKEYABLE	(long)0x10	/* cannot call up from "dmdld" */

#define MEMTEXT		(long)0x20	/* text section is allocated */
#define MEMDATA		(long)0x40	/* data section is allocated */
#define MEMBSS		(long)0x80	/* bss section is allocated */
#define MEMARGV		(long)0x100	/* argv array is allocated */
#define MEMCAPTION	(long)0x200	/* caption string is allocated */
#define MEMMENU		(long)0x400	/* menu structure is allocated */
#define MEMICON		(long)0x800	/* bitmap icon structure is allocated */


/* Fields arguments to the function cache(3L) */
#define A_TEXT		0x1		/* shared text */
#define A_NO_SHOW	0x2		/* do not show on menu */
#define A_BSS		0x4		/* do save original .bss section */
#define A_DATA		0x8		/* do save original .data section */
#define A_NO_BOOT	0x10		/* cannot boot from "dmdld" */
#define A_PERMANENT	0x20		/* cannot be removed from cache */

#define A_SHARED	(A_TEXT|A_DATA|A_BSS)	/* shared text */

Obj *realcache();
Obj *findobj();
Obj_user *objused();
void unreqcache();
