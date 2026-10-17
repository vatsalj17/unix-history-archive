	/*
	 *	ian j sep 77
	 *	additions by K.W.T.
	 *	/etc/warn [minutes to shutdown [minutes between warnings [message ...]]]
	 *
	 *	allow super users to tell users and remind users
	 *	of iminent shutdown of unix
	 */

	/*
	 *	Modified 14-11-78 to kill getty's and signal init
	 * 	thus preventing logging in after the message is sent.
	 *
	 *					Greg R and Bryan P.
	 */
#include	<utmp.h>
#include	<stat16.h>
#include	<param.h>
#include	<proc.h>
#include	<text.h>
#include	<inode.h>
struct {char d_minor; char d_major;};	/* for major and minor device numbers */

char	etcgetty[] "/etc/getty";
struct utmp buf;
struct statbuf	sbuf;
struct inode	ibuf;

struct text	tbuf;
int	kmemfd;
int mint 5;	/* assume five minutes between messages */
int mtogo 20;	/* assume twenty minutes to shutdown */
char term[] "/dev/tty?";


char	*message	"";
main(argc,argv)
int argc;
char **argv;

{
	register mtty,ufd;
	extern fout;
	struct proc	*p;
	int	maxprocs;
	char	**ap, *cp;

	if( argc > 1 ) mtogo = number(argv[1]);
	if( argc > 2 ) mint  = number(argv[2]);
	/*
	 * Coalesce the last arguments into one string 
	 * by replacing the nulls with blanks.
	 * This is a fudge, and I'm slightly ashamed...
	 * So much so that I won't put my name to it.
	 */
	if( argc > 3 )
	{
		message = argv[3];
		for(ap = &argv[3]; ap < &argv[argc-1]; ap++)
		{
			for(cp = *ap; *cp; cp++) ;
			*cp = ' ';
		}
	}
	if( mint >= mtogo )
	{
		printf("minutes to go(%d) <= minutes twixt(%d)\n",mtogo,mint);
		exit(1);
	}
	printf("\n %l minutes to system shut down\n",mtogo);
	printf(" %l minutes twixt user warnings\n",mint);
	/*
	 * find all gettys, and signal them with "14"s.
	 * also tell init.
	 *
	 * The way to find gettys is look up its inode structure,
	 * then hunt through kmem for a text structure matching
	 * this inode number. This works only if getty is compiled
	 * with "-n" or "-i" to be shareable.
	 */

	if(kill(1, 14) == -1)
	{
		perror("where is init?");
		exit(1);
	}
	/* open kmem, stat /etc/getty */
	if((kmemfd = open("/dev/kmem",0)) == -1)
	{
		perror("kmem");
		exit(1);
	}
	if(newstat(etcgetty, &sbuf) == -1)
	{
		perror(etcgetty);
		exit(1);
	}
	/* get proc table */
	if((maxprocs = gprocs(proc)) >= NPROC)
	{
		printf("%d procs in system - recompile warn!\n", maxprocs);
		exit(1);
	}
	/*
	 *	Now find the gettys and cause them to assume a state
	 *	of nonexistence.
	 */
	for(p = &proc[2]; p < &proc[maxprocs]; p++) /* ignore unix and init */
	{
		if(p->p_stat && isgetty(p))
			if(kill(p->p_pid, 14) == -1)
			{
				printf("%d:",p->p_pid);
				perror("cannot kill");
			}
	}

	clktim(mtogo*60 + 119);
	signal(15, 0);
	signal(14, 0);
	signal(2, 1);
	signal(3, 1);

	if(mtty = fork())
	{
		if( mtty < 0)
		{
			perror();
			exit(1);
		}
		exit(0);
	}
	mtty = ttyn(0);	/* note my tty id */
	for(;;)
	{
		ufd = open("/etc/utmp",0);
		if(ufd < 0)
		{
			perror("/etc/utmp");
			exit(1);
		}
		while( read(ufd,&buf,sizeof buf) == sizeof buf)
			if(buf.u_u_name[0])
			{
				term[8] = buf.u_ttyid;
				if( (fout = open(term,1)) > 0 )
				{
					printf("\007\007warning\n");
					printf("\007\007warning\n");
					if(mtogo > 0)
						printf("Log off. System going down in %d minute%s %s\n",
							mtogo, (mtogo > 1? "s": ""), message);
					else
						printf("LOG OFF. System going down NOW %s\n", message);
					printf("\007\007warning\n");
					printf("\007\007warning\n");
					flush();
					close(fout);
				}
			}
		close(ufd);
		sleep(mint*60);
		mtogo =- mint;
	}
}

number(s)
register char *s;

{
	register  n = 0;

	while( (*s>='0')&&(*s<='9') )  n = n*10 + *s++ - '0';
	return(n);
}


/*
 *	isgetty checks if the process pointer given it
 *	points to a getty.
 *	The algorithm used is to check the text pointer of each process
 *	until it finds a getty.
 *	kmem is read only twice for each unique text pointer until a getty
 *	is actually found.
 */
isgetty(p)
register struct proc *p;
{
	register int	**pp;
	static int	*gettyp, *notgp[NTEXT];

	if(p->p_textp == 0)
		return(0);	/* not shareable -- assume not getty */
	if(gettyp)
		return(gettyp == p->p_textp);
	for(pp = notgp; *pp; pp++)
		if(p->p_textp == *pp)
			return(0);
	/* pp points to first unused slot in notgp table
	** (note: overflow cannot happen)
	** and the inode number of this text slot must be determined.
	*/
	if(pp >= &notgp[NTEXT])
		abort();
	seek(kmemfd,p->p_textp,0);
	if(read(kmemfd, &tbuf, sizeof tbuf) != sizeof tbuf)
	{
		perror("reading text structure");
		exit(1);
	}
	seek(kmemfd, tbuf.x_iptr, 0);
	if(read(kmemfd, &ibuf, sizeof ibuf) != sizeof ibuf)
	{
		perror("reading inode");
		exit(1);
	}
	if(ibuf.i_dev.d_major == sbuf.sb_major
	 && ibuf.i_dev.d_minor == sbuf.sb_minor
	 && ibuf.i_number == sbuf.sb_inumber)
		gettyp = p->p_textp;
	else
		*pp = p->p_textp;
	return(isgetty(p));
}
