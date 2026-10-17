/*
 * Memory initialization and allocation; also parses arguments.
 */

#include "ilink.h"
#ifndef VMS
#ifndef ATT3B
#ifndef LATTICE
#include <sys/types.h>
#include <sys/stat.h>
#endif LATTICE
#endif ATT3B
#endif VMS

/*
 * Memory initialization
 */

struct gentry **ghash;		/* hash area for global table */
struct ientry **ihash;		/* hash area for identifier table */
struct fentry **fhash;		/* hash area for field table */

struct lentry *ltable;		/* local table */
struct gentry *gtable;		/* global table */
struct centry *ctable;		/* constant table */
struct ientry *itable;		/* identifier table */
struct fentry *ftable;		/* field table headers */
struct rentry *rtable;		/* field table record lists */

char *strings;			/* string space */
word  *labels;			/* label table */
char *code;			/* generated code space */

struct gentry *gfree;		/* free pointer for global table */
struct ientry *ifree;		/* free pointer for identifier table */
struct fentry *ffree;		/* free pointer for field table headers */
struct rentry *rfree;		/* free pointer for field table record lists */
char	      *strfree; 	/* free pointer for string space */
char	      *codep;		/* free pointer for code space */

word lsize = Lsize;		/* size of local table */
word gsize = GSize;		/* size of global table */
word csize = CSize;		/* size of constant table */
word isize = ISize;		/* size of identifier table */
word fsize = FSize;		/* size of field table headers */
word rsize = RSize;		/* size of field table record lists */
word ssize = StrSize;		/* size of string space */
word ghsize = GhSize;		/* size of global hash table */
word ihsize = IhSize;		/* size of identifier hash table */
word fhsize = FhSize;		/* size of field hash table */
word maxlabels = MaxLabels;	/* maximum number of labels per procedure */
word maxcode = MaxCode;		/* maximum amount of code per procedure */
int gmask;			/* mask for global table hash */
int imask;			/* mask for identifier table hash */
int fmask;			/* mask for field table hash */

char *ipath;

#ifdef VMS
#define MaxNAME 256
char	lclname[MaxNAME];	/* local name string for access()	*/
#endif VMS
#ifdef MSDOS
#define MaxNAME 256
char	lclname[MaxNAME];	/* local name string for access()	*/
#endif MSDOS

char *maknam();
char *maknam2();

/*
 * meminit - scan the command line arguments and initialize data structures.
 */
meminit(argc,argv)
int argc;
char **argv;
   {
   int aval;
   register int i;
   register union {
      struct gentry **gp;
      struct ientry **ip;
      struct fentry **fp;
      } p;
   extern char *allocate();
   extern char *instalid();
   extern char *getenv();
   extern struct gentry *putglob();

   lfiles = NULL;		/* Zero queue of files to link. */
   if ((ipath = getenv("IPATH")) == NULL)
#ifndef VMS
#ifndef MSDOS
      ipath = ".";              /* Just look in current directory if no IPATH. */
#else MSDOS
      ipath = ";";              /* Just look in current directory if no IPATH. */
#endif MSDOS
#else VMS
      ipath = "[]";             /* Just look in current directory if no IPATH. */
#endif VMS
   /*
    * Process the command line arguments.
    */
   while (--argc) {
      if (**++argv == '-') {
	 switch ((*argv)[1]) {
	    case 'm':           /* -m and -u are for the translator. */
	    case 'u':
	       continue;
	    case 't':           /* Set &trace to -1 when Icon starts up. */
	       trace = -1;
	       continue;
	    case 'D':           /* Produce a .ux file, which is a readable
				    version of the icode file produced. */
	       Dflag++;
	       continue;
	    case 'o':           /* Output file is next argument. */
		strcpy(outname,*++argv);
		argc--;
		continue;
#ifdef VMS
	    case 'r':
#endif VMS
	    case 'S':           /* Change some table size. */
	       if ((*argv)[3] == 'h') { /* Change hash table size. */
		  aval = atoi(&(*argv)[4]);
		  if (aval <= 0)
		     goto badarg;
		  switch ((*argv)[2]) {
		     case 'i': ihsize = aval; continue;
		     case 'g': ghsize = aval; continue;
		     case 'c': continue;
		     case 'f': fhsize = aval; continue;
		     case 'l': continue;
		     }
		  }
	       else {		/* Change symbol table size. */
		  aval = atoi(&(*argv)[3]);
		  if (aval <= 0)
		     goto badarg;
		  switch ((*argv)[2]) {
		     case 'c': csize = aval; continue;
		     case 'i': isize = aval; continue;
		     case 'g': gsize = aval; continue;
		     case 'l': lsize = aval; continue;
		     case 's': ssize = aval; continue;
		     case 't': continue;
		     case 'f': fsize = aval; continue;
		     case 'r': rsize = aval; continue;
		     case 'L': maxlabels = aval; continue;
		     case 'C': maxcode = aval; continue;
		     }
		  }
	    case 'i': {
	       iconx = *++argv;
	       argc--;
	       continue;
	       }
	    default:
	    badarg:
	       printf("bad argument: %s\n", *argv);
	       continue;
	    }
	 }
      else {		/* If not an argument, assume it's an input file. */
#ifndef VMS
#ifndef MSDOS
      if (access(*argv,4) != 0) {
#else MSDOS
      /*    *********** I don't know why this is here, addlfile
	   searches the paths for the file and determines if the file
	   can be read */
#endif MSDOS
#else VMS
      if ( access( maknam2(lclname,*argv,".u1"),4 ) != 0 ) {
#endif VMS
#ifndef MSDOS
	 fprintf(stderr, "ilink: cannot open %s\n", *argv);
	 exit(ErrorExit);
	    }
#endif MSDOS
	 addlfile(*argv);
	 }
      }

   /*
    * Round sizes of hash tables for locals, globals, constants, and
    *  identifiers to next larger power of two.  The corresponding
    *  mask values are set to one less than the hash table size so that
    *  an integer value can be &'d with the mask to produce a hash value.
    *  (See [lgc]hasher in sym.h.)
    */
   for (i = 1; i < ghsize; i <<= 1) ;
   ghsize = i;
   gmask = i - 1;
   for (i = 1; i < ihsize; i <<= 1) ;
   ihsize = i;
   imask = i - 1;
   for (i = 1; i < fhsize; i <<= 1) ;
   fhsize = i;
   fmask = i - 1;
   /*
    * Allocate the various data structures that are made on a per-file
    *  basis.
    */
   ghash   = (struct gentry **) allocate((int)ghsize, sizeof(struct gentry *));
   ihash   = (struct ientry **) allocate((int)ihsize, sizeof(struct ientry *));
   fhash   = (struct fentry **) allocate((int)fhsize, sizeof(struct fentry *));
   ltable  = (struct lentry *)	allocate((int)lsize, sizeof(struct lentry));
   gtable  = (struct gentry *)	allocate((int)gsize, sizeof(struct gentry));
   ctable  = (struct centry *)	allocate((int)csize, sizeof(struct centry));
   itable  = (struct ientry *)	allocate((int)isize, sizeof(struct ientry ));
   ftable  = (struct fentry *)	allocate((int)fsize, sizeof(struct fentry));
   rtable  = (struct rentry *)	allocate((int)rsize, sizeof(struct rentry));
   strings = (char *)		allocate((int)ssize, sizeof(char *));
   labels  = (word  *)		allocate((int)maxlabels, sizeof(word  *));
   code    = (char *)		allocate((int)maxcode, sizeof(char *));
   /*
    * Check to see if there was enough memory.	This assumes that the
    *  allocation for strings fails if any of the other allocations
    *  failed.	Apparent bug - that assumption is not necessarily valid.
    */

   if (code == NULL)
      syserr("can't get enough memory");
   /*
    * Reset the free pointer for each region.
    */
   gfree = gtable;
   ifree = itable;
   ffree = ftable;
   rfree = rtable;
   strfree = strings;
   codep = code;
   /*
    * Zero out the hash tables.
    */
   for (p.gp = ghash; p.gp < &ghash[ghsize]; p.gp++)
      *p.gp = NULL;
   for (p.ip = ihash; p.ip < &ihash[ihsize]; p.ip++)
      *p.ip = NULL;
   for (p.fp = fhash; p.fp < &fhash[fhsize]; p.fp++)
      *p.fp = NULL;
   /*
    * Install "main" as a global variable in order to insure that it
    *  is the first global variable.  iconx/start.s depends on main
    *  being global number 0.
    */
   putglob(instalid("main"), F_Global, 0);
   }

/*
 * allocate - get more memory from system.
 */
char *allocate(n, size)
int n, size;
   {
#ifndef MSDOS
   return (char *)malloc(n * size);
#else MSDOS
   return (char *)calloc(n,size);
#endif MSDOS
   }

/*
 * alclfile - allocate an lfile structure for the named file, fill
 *  in the name and return a pointer to it.
 */
struct lfile *alclfile(name)
char *name;
   {
   struct lfile *p;
   char *np;
   int l;

   p = (struct lfile *)allocate(1,sizeof(struct lfile));
   if (!p)
      syserr("not enough memory for file list");
   p->lf_link = NULL;
   l = strlen(name);
   np = allocate(1,(l+1+sizeof(int *)) & ~(sizeof(int *)-1));
   if (!np)
      syserr("not enough memory for file list");
   strncpy(np,name,l);
   p->lf_name = np;
   return p;
   }

/*
 * dumplfiles - print the list of files to link.  Used for debugging only.
 */

dumplfiles()
   {
   struct lfile *p,*lfls;

   printf("lfiles:\n");
   lfls = lfiles;
   while (p = getlfile(&lfls))
       printf("'%s'\n",p->lf_name);
   }
/*
 * addlfile - create an lfile structure for the named file and add it to the
 *  end of the list of files (lfiles) to generate link instructions for.
 */
char *pptr;
addlfile(name)
char *name;
   {
   struct lfile *nlf, *p;
   char file[256], ok;

#ifndef VMS
#ifndef MSDOS
   if (index(name,'/') == 0) {
#else MSDOS
   if (index(name,'/') == 0 &&
       index(name,'\\') == 0 &&
       index(name,':') == 0) {
#endif MSDOS
#else VMS
   if (strchr(name,']') == 0 && strchr(name,':') == 0) {
#endif VMS
      pptr = ipath;
      ok = 0;
#ifdef MSDOS
      strcpy(file,name);
      if (canread(file)) ok++; /* See if we can read the file here */
      else
#endif MSDOS
      while (trypath(name,file)) {
	 if (canread(file)) {
	    ok++;
	    break;
	    }
	 }
      if (!ok) {
	 fprintf(stderr, "Can't resolve reference to file '%s'\n",name);
	 exit(ErrorExit);
	 }
      }
   else
      strcpy(file,name);
   nlf = alclfile(file);
   if (lfiles == NULL) {
      lfiles = nlf;
      }
   else {
      p = lfiles;
      while (p->lf_link != NULL) {
	 if (strcmp(p->lf_name,file) == 0)
	    return;
	 p = p->lf_link;
	 }
      if (strcmp(p->lf_name,file) == 0)
	 return;
      p->lf_link = nlf;
      }
   }

/*
 * getlfile - return a pointer (p) to the lfile structure pointed at by lptr
 *  and move lptr to the lfile structure that p points at.  That is, getlfile
 *  returns a pointer to the current (wrt. lptr) lfile and advances lptr.
 */
struct lfile *
getlfile(lptr)
struct lfile **lptr;
   {
   struct lfile *p;

   if (*lptr == NULL)
      return NULL;
   else {
      p = *lptr;
      *lptr = p->lf_link;
      return p;
      }
   }

/*
 * canread - see if file can be read and be sure that it's just an
 *  ordinary file.
 */
canread(file)
char *file;
   {
#ifndef VMS
#ifndef LATTICE
   struct stat statb;
#endif LATTICE
#endif VMS

#ifndef VMS
#ifndef LATTICE
   if (access(file,4) == 0) {
      stat(file,&statb);
      if (statb.st_mode & S_IFREG)
	 return 1;
      }
#else LATTICE
   if (access( maknam2(lclname,file,".u1"), 4 ) == 0 )
      return 1;
#endif LATTICE
#else VMS
   if (access( maknam2(lclname,file,".u1"), 4 ) == 0 )
      return 1;
#endif VMS

   return 0;
   }

/*
 * trypath - form a file name in file by concatenating name onto the
 *  next path element.
 */
trypath(name,file)
char *name, *file;
   {
   char *n, c;

#ifndef VMS
#ifndef MSDOS
   while (*pptr == ':')
#else MSDOS
   while (*pptr ==';')
#endif MSDOS
#else VMS
   while (*pptr == ' ')
#endif VMS
      pptr++;
   if (!*pptr)
      return 0;
   do {
      c = (*file++ = *pptr++);
#ifndef VMS
#ifndef MSDOS
      } while (c != ':' && c);
#else MSDOS
      } while (c != ';' && c);
#endif MSDOS
#else VMS
      } while (c != ' ' && c);
#endif VMS
   pptr--;
   file--;

#ifndef VMS
   *file++ = '/';
#endif VMS
   while (*file++ = *name++);
   *file = 0;
   return(1);
   }
