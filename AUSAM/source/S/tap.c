#
/* tap [-fildes] pid:
 *
 *	display the contents of an active pipe on the fildes
 *	defaults to fildes 1 if no fildes is specified.  Must be
 *	run as a set-user-id to root program.
 *
 *					Written By Bryan Palmer.
 */ 

#include <local-system>
#include <param.h>
#include <user.h>
#include <proc.h>
#include <file.h>
#include <stat16.h>
#define	NINODE	1
#include <inode.h>
#include <stdio.h>

#define STANDOUT 1
#define NOTNICE -30
#define NICE 0

char usage[] "Usage: tap [-fildes] pid ...\n";	/* cretin message */ 
char unix[] "/unix";				/* unix lives here */ 
char mem[] "/dev/mem";				/* memory */ 
char swap[] "/dev/swap";			/* swap disk */ 
char dev[] "/dev";				/* where the devices live */ 
char root[] "/";				/* root device */

int memfd;					/* memory fildes */ 
int swapfd;					/* swap fildes */ 
int rootfd;					/* root fildes */

int ruid;					/* my uid */ 

struct nll
{
	int n_name[4];
	int n_type;
	int n_value;
} nll[]
{

	'_f', 'il', 'e\0', '\0\0', 0, 0,
	0,
};

main(cookyc, vcooky)
register char **vcooky;
register int cookyc;
{
	register int fildes;


	cookyc--, vcooky++;

	/* find if the person wants to look at a
	 * different fildes to standard out (1)
	 * and change the variable fildes to allow
	 * this to happen.
	 */ 
	if(**vcooky == '-')
	{
		fildes = atoi(&vcooky[0][1]);
		if((fildes < 0) || (fildes > NOFILE))
		{
			fprintf(stderr, usage);
			fprintf(stderr, "fildes defaulted to 1\n\n");
			fildes = STANDOUT;
		}
		cookyc--, vcooky++;
	}
	else
		fildes = STANDOUT;


	/* do a nlist on unix to
	 * obtain the addresses of 
	 * the file structures
	 */
	nlist(unix, nll);
	if(nll[0].n_value == 0)
	{
		fprintf(stderr, "Cant do a nlist for \"%s\" in \"%s\"\n",
			nll[0].n_name, unix);
		exit(1);
	}

	/* open up the root device so */
	getrootdev();

	/* open the swap space and
	 * the memory for future reading
	 */ 
	if((memfd = open(mem, 0)) == -1)
	{
		perror(mem);
		exit(1);
	}

	if((swapfd = open(swap, 0)) == -1)
	{
		perror(swap);
		exit(2);
	}

	ruid = getreal();

	if(cookyc)
	{
		do
		{
			tap(fildes, atoi(*vcooky));
			vcooky++;
		}
		while(--cookyc);
	}
	else
	{
		fprintf(stderr, usage);
		exit(1);
	}

	exit(0);
}

struct proc *getslot(pid)
register int pid;
{
	register struct proc *ppr;
	register int i;
	int hiwater;


	ppr = &proc[2];	/* skip past unix and init */ 
	hiwater = gprocs(proc);
	for(i = 2; i < hiwater; i++)
	{
		/* find the slot containing the process
		 * and make sure that the person has permission
		 * to read the pipe
		 */ 
		if((ppr->p_stat) && (ppr->p_pid == pid) && (ppr->p_stat != SZOMB) && ((ppr->p_uid == ruid) || (ruid == 0)))
			return(ppr);
		ppr++;
	}

	return(0);	/* proc not found */ 

}

getuser(pptr, fildes)
register struct proc *pptr;
{
	register int device;
	long position, tmp;
	struct user u;
	int ff;



	position = &0->u_ofile[fildes];
	tmp = pptr->p_addr;
	if(pptr->p_flag & SLOAD)
	{
		/* user stucture is in core */ 
		device = memfd;
		position =+ tmp<<6;
	}
	else
	{
		/* user structure on swap space */ 
		device = swapfd;
		position =+ tmp * 512;
	}

	/* seek to the address of the appropriate file
	 * file structure and return it
	 */
	if(lseek(device, position, 0) == -1)
	{
		perror("lseek error");
		return(-1);
	}
	if(read(device, &ff, 2) != 2)
	{
		perror("read error");
		return(-1);
	}

	return(ff);
}


tap(fildes, pid)
int fildes, pid;
{
	register struct proc *procptr;
	register int ff;
	struct file ffle;	/* file struct */ 

	/* the reading of the proc table and the
	 * locating of the user structure must be niced
	 * to guarentee some sort of result.  The nice
	 * is turned off when this area is passed.
	 */ 


	nice(NOTNICE);	/* aid the seeking of the user structure */ 
	if((procptr = getslot(pid)) == 0)
	{
		nice(NICE);
		fprintf(stderr, "%d: process not found\n", pid);
		return(-1);
	}
	if((ff = getuser(procptr, fildes)) == -1)
	{
		nice(NICE);
		fprintf(stderr, "%d: User structure not found?\n", pid);
		return(-2);
	}
	nice(NICE);	/* niceness no longer needed */ 

	/* seek to the location of the desired file
	 * structure to obtain the inode.  Also check
	 * that the file is really a pipe and check that
	 * the file is really open.
	 */ 

	if(ff == 0)
	{
		/* file not open */ 
		fprintf(stderr, "%d: file descriptor not open\n", fildes);
		return(-8);
	}

	/* locate file structure */ 
	if(seek(memfd, ff, 0) == -1)
	{
		perror(mem);
		return(-3);
	}
	/* read in file structure */ 
	if(read(memfd, &ffle, sizeof ffle) == -1)
	{
		perror(mem);
		return(-4);
	}
	/* check 'is pipe?' */ 
	if(!(ffle.f_flag & FPIPE))
	{
		fprintf(stderr, "%d: not a pipe\n", fildes);
		return(-3);
	}

	/* locate inode */ 
	if(seek(memfd, ffle.f_inode, 0) == -1)
	{
		perror(mem);
		return(-5);
	}

	/* read in inode */ 
	if(read(memfd, &inode, sizeof inode[0]) != sizeof inode[0])
	{
		perror(mem);
		return(-5);
	}

	/* sort the pipe out and write the contents */ 
	if(printaddrs(ffle.f_inode, pid, fildes) == -1)
	{
		fprintf(stderr, "%d: pipe now closed\n", pid);
		return(-1);
	}
	writepipe(pid, &inode[0]);
}

getrootdev()
{

	struct direct
	{
		int d_inode;
		char d_name[14];
	} direct;
	register struct direct *dd;
	int fd;
	register char *ch;
	struct statbuf sb, se;
	int  devnum;

	/* stat '/' to obtain the major and minor device
	 * numbers of the root device
	 */

	if(stat(root,&se) == -1)
	{
		perror(root);
		exit(1);
	}
	devnum = (se.sb_major << 8) + se.sb_minor;

	/* chdir to /dev than search thru the directory
	 * looking for the root device
	 * Then attempt to open it as a raw device.  If this fails
	 * open it as a cooked device.  If this fails rack off
	 */ 


	if(chdir(dev) == -1)
	{
		perror(dev);
		exit(1);
	}

	/* open '.' */ 
	if((fd = open("", 0)) == -1)
	{
		perror(dev);
		exit(1);
	}

	dd = &direct;
	while(read(fd, dd, 16) == 16)
	{
		if((dd->d_inode) && (stat(dd->d_name, &sb) != -1) && (sb.sb_addr[0] == devnum) && ((sb.sb_flags & IFTYP) == IFBLK))
		{
			close(fd);
			if((rootfd = open(dd->d_name, 0)) == -1)
			{
				perror(dd->d_name);
				exit(1);
			}
			return(0);
		}
	}

	fprintf(stderr, "No root device??\n");
	exit(1);

}

writepipe(pid, inp)
int pid;
register struct inode *inp;
{
	char buff[512];
	register int i;

	fflush(stdout);	/* cause stdio to sync with hard wire io */ 
	for(i = 0; i < 8; i++)
	{
		if(inp->i_addr[i])
		{
			if(seek(rootfd, inp->i_addr[i], 3) != -1)
			{
				read(rootfd, buff, sizeof buff);
				write(1, buff, sizeof buff);
			}
		}
	}
	fprintf(stdout, "\n\n");
}

printaddrs(inp, pid, fildes)
struct inode *inp;
int pid, fildes;
{
	long readptr, writeptr;
	int rchk, wchk;
	register int i;


	rchk = wchk = 0;
	seek(memfd, nll[0].n_value, 0);	/* seek to file structure */ 
	read(memfd, file, sizeof file);
	for(i = 0; i < NFILE; i++)
	{
		if(file[i].f_inode == inp)
		{
			if(file[i].f_flag & FREAD)
			{
				readptr = file[i].f_offset;
				rchk++;
			}
			else if(file[i].f_flag & FWRITE)
			{
				writeptr = file[i].f_offset;
				wchk++;
			}
		}
	}
	fprintf(stdout, "Proc: %d		Fildes: %d\n", pid, fildes);
	if(rchk)
		fprintf(stdout, "Read  pointer: %D\n", readptr);
	if(wchk)
		fprintf(stdout, "Write pointer: %D\n", writeptr);
	if((rchk == 0) && (wchk == 0))
		return(-1);
	return(0);
}
