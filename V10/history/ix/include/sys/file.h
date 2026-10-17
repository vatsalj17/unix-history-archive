#ifndef FREAD
/*
 * One file structure is alloted
 * for each open/creat/pipe call.
 * Main use is to hold the read/write
 * pointer associated with each open
 * file.
 */
struct	file
{
	short	f_flag;
	short	f_count;		/* reference count */
	struct inode *f_inode;		/* pointer to inode structure */
	llong_t	f_offset;		/* read/write character pointer */
	struct	jlabel *f_lab;		/* security label of f_offset */
	struct 	fildes *f_fref;		/* all refs to this file */
};

/*
 * One file ref structure per open file in each process.
 * Pointers everywhich way for rapid access of other processes
 * with same file structure, etc.
 */
struct fildes
{
	short		fr_pofile;
	short		fr_fd;		/* for newproc() */
	struct proc	*fr_proc;
	struct file	*fr_file;
	struct fildes	*fr_nf;		/* another with same file structure */
	struct fildes	*fr_bf;		/* backwards */
	struct fildes	*fr_ni;		/* ditto, same inode */
	struct fildes	*fr_bi;		
	struct fildes	*fr_np;		/* ditto, same process */
	struct fildes	*fr_bp;
};

#ifdef	KERNEL
extern struct	file file[];
extern struct file *fileNFILE;
extern int filecnt;

extern struct	fildes fildes[];
extern int fildescnt;

struct	file *allocfile();
struct fildes *f_glue();


/*
 * inline to speed things up
 */
#define	getf(fd)	((unsigned)(fd)>=NOFILE ? NULL :\
	((u.u_ofile[fd])? (u.u_ofile[fd])->fr_file : NULL))
struct inode *geti();

#endif


/* flags in f_flags */
#define	FREAD	01
#define	FWRITE	02
#define	FHUNGUP 010

/* codes in fr_pofile .  also T_NOCHK in label.h */
#define 	EXCLOSE 	01
#define 	SAFETOREAD 	02
#define		SAFETOWRITE	010
#define		DIDREAD		020
#define		DIDWRITE	040
#define		DIDRAISE	0200
#endif
