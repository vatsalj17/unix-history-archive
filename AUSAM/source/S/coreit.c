#

/*
 * prints out a brief summary of the status
 * of a core dump.
 */

#include <param.h>
#include <file.h>
#include <user.h>
#include <reg.h>

struct	{
	char	*rname;
	int	rnum;
	} regs[9] {
	"R0",	R0,
	"R1",	R1,
	"R2",	R2,
	"R3",	R3,
	"R4",	R4,
	"R5",	R5,
	"SP",	R6,
	"PC",	R7,
	"PS",	RPS
	};

char	*sigmsg[NSIG] {
	"0",
	"hangup",
	"interrupt",
	"quit",
	"illegal instruction",
	"trace/breakpoint",
	"iot",
	"emt",
	"floating exception",
	"kill",
	"bus error",
	"segmentation violation",
	"bad system call",
	"broken pipe",
	"terminated",
	"real-time limit",
	"cpu-time limit",
	"17",
	"18",
	"19"
	};

struct	{
	int	f_dummy;
	double	ac0,ac4,ac5,ac1,ac2,ac3;
	};

int	thebuff[1024/2];

int	inode[18];

char	abuff[512];	/* for argument evaluation */

main(argc, argv)
int	argc;
char	*argv[];
{
	int	i, stuffed;

	stuffed = 0;
	if (argc < 2) {
		if (dumpc("core") < 0)
			stuffed++;
	} else
		for (i = 1; i < argc; i++)
			if (dumpc(argv[i]) < 0) {
				stuffed++;
				break;
			}
	flush();
	exit(stuffed != 0);
}

dumpc(afile)
char	*afile;
{
	int	fid;
	register struct user	*p;
	register int	i;
	int	flag;
	long	fsize;
	struct {
		int	long2[2];
		};

	p = thebuff;

	if (stat(afile, inode) < 0) {
		printf("no such file - %s\n", afile);
		return(-1);
	}
	if (inode[2] & 060000) {
		printf("not an ordinary file - %s\n", afile);
		return(-1);
	}

	if ((fid = open(afile, 0)) < 0) {
		printf("can't open file - %s\n", afile);
		return(-1);
	}
	if (read(fid, p, 1024) != 1024) {
		close(fid);
		printf("invalid file - %s\n", afile);
		return(-1);
	}

	fsize.long2[0] = (inode[4] >> 8) & 0377;
	fsize.long2[1] = inode[5];
	i = p->u_dsize + p->u_ssize;
	if (p->u_flags & USHRDATA)
		i =+ p->u_tsize;	/* shared data */
	i =+ (1024 >> 6);
	if ((fsize >> 6) != i) {
		printf("invalid core file : %s\n", afile);
		return(-1);
	}

	printf("\n--> file: %s <--\n\n", afile);

	if (anal(fid) < 0)
		return(-1);
	close(fid);


	if (p->u_tsize != 0)
		printf("text size = %o bytes\n", p->u_tsize<<6);
	if (p->u_dsize != 0)
		printf("data size = %o bytes\n", p->u_dsize<<6);
	printf("stack size = %o bytes\n", p->u_ssize<<6);
	if (p->u_sep)
		printf("separated I/D spaces\n");
	if (p->u_flags & USHRDATA)
		printf("shared data segment\n");

	printf("Reason for dump: %s\n", sigmsg[p->u_arg[0]]);

	printf("user id: (real)=%o, (effective)=%o\n", p->u_uid, p->u_ruid);

	printf("system time usage = %D. ticks\n", p->u_stime);
	printf("user time usage = %D. ticks\n", p->u_utime);
	if (p->u_cstime!=0 || p->u_cutime!=0) {
		printf("sum of childrens...:\n");
		printf("  system time = %D. ticks\n", p->u_cstime);
		printf("  user time = %D. ticks\n", p->u_cutime);
	}

	if (p->u_prof[3] != 0) {
		printf("profiling enabled:\n");
		printf("  buffer = %o\n", p->u_prof[0]);
		printf("  buff size = %o\n", p->u_prof[1]);
		printf("  offset = %o\n", p->u_prof[2]);
		printf("  scale = %o\n", p->u_prof[3]);
	}

	flag = 0;
	for (i = 0; i < NOFILE; i++)
		if (p->u_ofile[i] != 0) {
			if (!flag) {
				printf("open files:");
				printf(" (filedescriptor and &inode)\n");
				flag++;
			} else
				printf(", ");
			printf("%d. - %o", i, p->u_ofile[i]);
		}
	if (!flag)
		printf("no open files\n");
	else
		printf("\n");

/*
	printf("prototype segmentation registers:\n");
	for (i = 0; i < 8; i++)
		if (p->uisd[i] & 060) {
			printf(" PAR/PDR %d", i);
			printf(" %6o", p->u_uisa[i]);
			printf(" %3o", (p->u_uisd[i] >> 8) & 0377);
			printf(" %s\n", ((p->u_uisd[i] & 060) == 060) ? "rw" : "ro");
		}
 */

	printf("system call arguments:\n");
	for (i = 0; i < 5; i++)
		printf(" %o", p->u_arg[i]);
	printf("\n");

	printf("registers:\n");
	i = p;
	i =+ p->u_ar0;
	i =- 0140000;
	p->u_ar0 = i;
	for (i = 0; i < 9; i++)
		printf(" %s %o\n", regs[i].rname, p->u_ar0[regs[i].rnum]);
#ifdef	FPU
	printf("Floating point registers:\n");
		printf("FEC = %o, FEA = %o, FPS = %o\n",
			p->u_fec, p->u_fea, p->u_fsav[0]);
		printf("AC0 = %24.18e\n", p->u_fsav->ac0);
		printf("AC1 = %24.18e\n", p->u_fsav->ac1);
		printf("AC2 = %24.18e\n", p->u_fsav->ac2);
		printf("AC3 = %24.18e\n", p->u_fsav->ac3);
		printf("AC4 = %24.18e\n", p->u_fsav->ac4);
		printf("AC5 = %24.18e\n", p->u_fsav->ac5);

#endif	FPU
	printf("signals:\n");
	for (i = 0; i < NSIG; i++)
		if (p->u_signal[i] != 0) {
			printf("  %s:  ", sigmsg[i]);
			if (p->u_signal[i] & 01)
				printf("ignored\n");
			else
				printf("trap to - %o\n", p->u_signal[i]);
		}


	return(0);
}

anal(afid)
int	afid;
{
	register int	*ip;
	register char	*cp;

	seek(afid, -512, 2);
	if (read(afid, abuff, 512) != 512) {
		printf("Can't read arguments from file?\n");
		return(-1);
	}
	ip = &abuff[512];
	while (*--ip != -1)
		if (ip == abuff) {
			printf("** File exec'ed with no arguments.\n");
			return(0);
		}
	cp = ip + 1;
	if (*cp == '\0')
		cp++;
	printf("name - %s\n", cp);
	return(0);
}
