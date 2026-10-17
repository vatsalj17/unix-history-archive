#ifndef lint
static	char *sccsid = "@(#)pstat.c 1.1 85/05/30 SMI"; /* from UCB 4.22 */
#endif
/*
 * Print system stuff
 */

#define mask(x) (x&0377)
#define	clear(x) ((int)x&0x7fffffff)

#include <sys/param.h>
#include <sys/dir.h>
#define	KERNEL
#include <sys/file.h>
#undef	KERNEL
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/text.h>
#include <sys/vnode.h>
#include <ufs/inode.h>
#include <sys/map.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/conf.h>
#include <sys/vm.h>
#include <nlist.h>
#include <machine/pte.h>

char	*fcore	= "/dev/kmem";
char	*fnlist	= "/vmunix";
int	fc;

struct nlist nl[] = {
#define	SINODE	0
	{ "_inode" },
#define	STEXT	1
	{ "_text" },
#define	SPROC	2
	{ "_proc" },
#define	SDZ	3
	{ "_dz_tty" },
#define	SNDZ	4
	{ "_dz_cnt" },
#define	SKL	5
	{ "_cons" },
#define	SFIL	6
	{ "_file" },
#define	USRPTMA	7
	{ "_Usrptmap" },
#define	USRPT	8
	{ "_usrpt" },
#define	SNSWAP	9
	{ "_nswap" },
#define	SWAPMAP	10
	{ "_swapmap" },
#define	SDH	11
	{ "_dh11" },
#define	SNDH	12
	{ "_ndh11" },
#define	SNPROC	13
	{ "_nproc" },
#define	SNTEXT	14
	{ "_ntext" },
#define	SNFILE	15
	{ "_nfile" },
#define	SNINODE	16
	{ "_ninode" },
#define	SNSWAPMAP 17
	{ "_nswapmap" },
#define	SPTY	18
	{ "_pt_tty" },
#define	SDMMIN	19
	{ "_dmmin" },
#define	SDMMAX	20
	{ "_dmmax" },
#define	SNSWDEV	21
	{ "_nswdev" },
	0,
};

int	inof;
int	txtf;
int	prcf;
int	ttyf;
int	usrf;
long	ubase;
int	filf;
int	swpf;
int	totflg;
char	partab[1];
struct	cdevsw	cdevsw[1];
struct	bdevsw	bdevsw[1];
int	allflg;
int	kflg;
struct	pte *Usrptma;
struct	pte *usrpt;

main(argc, argv)
char **argv;
{
	register char *argp;
	int allflags;

	argc--, argv++;
	while (argc > 0 && **argv == '-') {
		argp = *argv++;
		argp++;
		argc--;
		while (*argp++)
		switch (argp[-1]) {

		case 'T':
			totflg++;
			break;

		case 'a':
			allflg++;
			break;

		case 'i':
			inof++;
			break;

		case 'k':
			kflg++;
			fcore = "/vmcore";
			break;

		case 'x':
			txtf++;
			break;

		case 'p':
			prcf++;
			break;

		case 't':
			ttyf++;
			break;

		case 'u':
			if (argc == 0)
				break;
			argc--;
			usrf++;
			sscanf( *argv++, "%x", &ubase);
			break;

		case 'f':
			filf++;
			break;
		case 's':
			swpf++;
			break;
		default:
			usage();
			exit(1);
		}
	}
	if (argc>1)
		fcore = argv[1];
	if ((fc = open(fcore, 0)) < 0) {
		printf("Can't find %s\n", fcore);
		exit(1);
	}
	if (argc>0)
		fnlist = argv[0];
	nlist(fnlist, nl);
	usrpt = (struct pte *)nl[USRPT].n_value;
	Usrptma = (struct pte *)nl[USRPTMA].n_value;
	if (nl[0].n_type == 0) {
		printf("no namelist\n");
		exit(1);
	}
	allflags = filf | totflg | inof | prcf | txtf | ttyf | usrf | swpf;
	if (allflags == 0) {
		printf("pstat: one or more of -[aixptfsu] is required\n");
		exit(1);
	}
	if (filf||totflg)
		dofile();
	if (inof||totflg)
		doinode();
	if (prcf||totflg)
		doproc();
	if (txtf||totflg)
		dotext();
	if (ttyf)
		dotty();
	if (usrf)
		dousr();
	if (swpf||totflg)
		doswap();
	exit(0);
}

usage()
{

	printf("usage: pstat -[aixptfs] [-u [ubase]] [system] [core]\n");
}

doinode()
{
	register struct inode *ip;
	register struct vnode *vp;
	struct inode *xinode, *ainode;
	register int nin;
	int ninode;

	nin = 0;
	ninode = getw(nl[SNINODE].n_value);
	xinode = (struct inode *)calloc(ninode, sizeof (struct inode));
	lseek(fc, (int)(ainode = (struct inode *)getw(nl[SINODE].n_value)), 0);
	read(fc, xinode, ninode * sizeof(struct inode));
	for (ip = xinode; ip < &xinode[ninode]; ip++)
		if (ip->i_vnode.v_count)
			nin++;
	if (totflg) {
		printf("%3d/%3d inodes\n", nin, ninode);
		return;
	}
	printf("%d/%d active inodes\n", nin, ninode);
printf("  ILOC   IFLAG   IDEVICE   INO   MODE NLK  UID  SIZE/DEV VFLAG CNT SHC EXC TYPE\n");
	for (ip = xinode; ip < &xinode[ninode]; ip++) {
		vp = &ip->i_vnode;
		if (vp->v_count == 0)
			continue;
		printf("%8.1x ", ainode + (ip - xinode));
		putf(ip->i_flag&IACC, 'A');
		putf(ip->i_flag&ICHG, 'C');
		putf(ip->i_flag&ILOCKED, 'L');
		putf(ip->i_flag&IREF, 'R');
		putf(ip->i_flag&ITEXT, 'T');
		putf(ip->i_flag&IUPD, 'U');
		putf(ip->i_flag&IWANT, 'W');
		putf(ip->i_flag&ILWAIT, 'Z');
		printf("%3d,%3d", major(ip->i_dev), minor(ip->i_dev));
		printf("%6d", ip->i_number);
		printf("%7o", ip->i_mode & 0xffff);
		printf("%4d", ip->i_nlink);
		printf("%5d", ip->i_uid);
		if ((ip->i_mode&IFMT)==IFBLK || (ip->i_mode&IFMT)==IFCHR)
			printf("%6d,%3d", major(ip->i_rdev), minor(ip->i_rdev));
		else
			printf("%10ld", ip->i_size);
		printf(" ");
		putf(vp->v_flag&VROOT, 'R');
		putf(vp->v_flag&VSHLOCK, 'S');
		putf(vp->v_flag&VEXLOCK, 'E');
		putf(vp->v_flag&VTEXT, 'T');
		putf(vp->v_flag&VLWAIT, 'Z');
		printf("%4d", vp->v_count&0377);
		printf("%4d", vp->v_shlockc&0377);
		printf("%4d", vp->v_exlockc&0377);
		switch (vp->v_type) {
			case VNON:
				printf(" VNON");
				break;
			case VREG:
				printf(" VREG");
				break;
			case VDIR:
				printf(" VDIR");
				break;
			case VBLK:
				printf(" VBLK");
				break;
			case VCHR:
				printf(" VCHR");
				break;
			case VLNK:
				printf(" VLNK");
				break;
			case VSOCK:
				printf(" VSOC");
				break;
			case VBAD:
				printf(" VBAD");
				break;
			default:
				printf(" ????");
				break;
		}
		printf("\n");
	}
	free(xinode);
}

getw(loc)
	off_t loc;
{
	int word;

	if (kflg)
		loc &= 0x7fffffff;
	lseek(fc, loc, 0);
	read(fc, &word, sizeof (word));
	if (kflg)
		word &= 0x7fffffff;
	return (word);
}

putf(v, n)
{
	if (v)
		printf("%c", n);
	else
		printf(" ");
}

dotext()
{
	register struct text *xp;
	int ntext;
	struct text *xtext, *atext;
	int ntx;

	ntx = 0;
	ntext = getw(nl[SNTEXT].n_value);
	xtext = (struct text *)calloc(ntext, sizeof (struct text));
	lseek(fc, (int)(atext = (struct text *)getw(nl[STEXT].n_value)), 0);
	read(fc, xtext, ntext * sizeof (struct text));
	for (xp = xtext; xp < &xtext[ntext]; xp++)
		if (xp->x_vptr!=NULL)
			ntx++;
	if (totflg) {
		printf("%3d/%3d texts\n", ntx, ntext);
		return;
	}
	printf("%d/%d active texts\n", ntx, ntext);
	printf("   LOC   FLAGS DADDR      CADDR  RSS SIZE      VPTR  CNT CCNT\n");
	for (xp = xtext; xp < &xtext[ntext]; xp++) {
		if (xp->x_vptr == NULL)
			continue;
		printf("%8.1x", atext + (xp - xtext));
		printf(" ");
		putf(xp->x_flag&XPAGV, 'P');
		putf(xp->x_flag&XTRC, 'T');
		putf(xp->x_flag&XWRIT, 'W');
		putf(xp->x_flag&XLOAD, 'L');
		putf(xp->x_flag&XLOCK, 'K');
		putf(xp->x_flag&XWANT, 'w');
		printf("%5x", xp->x_daddr[0]);
		printf("%11x", xp->x_caddr);
		printf("%5d", xp->x_rssize);
		printf("%5d", xp->x_size);
		printf("%10.1x", xp->x_vptr);
		printf("%5d", xp->x_count&0377);
		printf("%5d", xp->x_ccount);
		printf("\n");
	}
	free(xtext);
}

doproc()
{
	struct proc *xproc, *aproc;
	int nproc;
	register struct proc *pp;
	register loc, np;
	struct pte apte;

	nproc = getw(nl[SNPROC].n_value);
	xproc = (struct proc *)calloc(nproc, sizeof (struct proc));
	lseek(fc, (int)(aproc = (struct proc *)getw(nl[SPROC].n_value)), 0);
	read(fc, xproc, nproc * sizeof (struct proc));
	np = 0;
	for (pp=xproc; pp < &xproc[nproc]; pp++)
		if (pp->p_stat)
			np++;
	if (totflg) {
		printf("%3d/%3d processes\n", np, nproc);
		return;
	}
	printf("%d/%d processes\n", np, nproc);
	printf("   LOC    S    F POIP PRI      SIG  UID SLP TIM  CPU  NI   PGRP    PID   PPID    ADDR   RSS SRSS SIZE    WCHAN    LINK   TEXTP CLKT\n");
	for (pp=xproc; pp<&xproc[nproc]; pp++) {
		if (pp->p_stat==0 && allflg==0)
			continue;
		printf("%8x", aproc + (pp - xproc));
		printf(" %2d", pp->p_stat);
		printf(" %4x", pp->p_flag & 0xffff);
		printf(" %4d", pp->p_poip);
		printf(" %3d", pp->p_pri);
		printf(" %8x", pp->p_sig);
		printf(" %4d", pp->p_uid);
		printf(" %3d", pp->p_slptime);
		printf(" %3d", pp->p_time);
		printf(" %4d", pp->p_cpu&0377);
		printf(" %3d", pp->p_nice);
		printf(" %6d", pp->p_pgrp);
		printf(" %6d", pp->p_pid);
		printf(" %6d", pp->p_ppid);
		if (kflg)
			pp->p_addr = (struct pte *)clear((int)pp->p_addr);
		lseek(fc, (long)(Usrptma+btokmx(pp->p_addr)), 0);
		read(fc, &apte, sizeof(apte));
		printf(" %8x", ctob(apte.pg_pfnum+1) - sizeof(struct pte) * UPAGES);
		printf(" %4x", pp->p_rssize);
		printf(" %4x", pp->p_swrss);
		printf(" %5x", pp->p_dsize+pp->p_ssize);
		printf(" %7x", clear(pp->p_wchan));
		printf(" %7x", clear(pp->p_link));
		printf(" %7x", clear(pp->p_textp));
		printf("\n");
	}
}

dotty()
{
	struct tty dz_tty[128];
	int ndz;
	register struct tty *tp;
	register char *mesg;

	printf("1 cons\n");
	if (kflg)
		nl[SKL].n_value = clear(nl[SKL].n_value);
	lseek(fc, (long)nl[SKL].n_value, 0);
	read(fc, dz_tty, sizeof(dz_tty[0]));
	mesg = " # RAW CAN OUT   MODE    ADDR   DEL COL  STATE   PGRP DISC\n";
	printf(mesg);
	ttyprt(&dz_tty[0], 0);
	if (nl[SNDZ].n_type == 0)
		goto dh;
	if (kflg) {
		nl[SNDZ].n_value = clear(nl[SNDZ].n_value);
		nl[SDZ].n_value = clear(nl[SDZ].n_value);
	}
	lseek(fc, (long)nl[SNDZ].n_value, 0);
	read(fc, &ndz, sizeof(ndz));
	printf("%d dz lines\n", ndz);
	lseek(fc, (long)nl[SDZ].n_value, 0);
	read(fc, dz_tty, ndz * sizeof (struct tty));
	for (tp = dz_tty; tp < &dz_tty[ndz]; tp++)
		ttyprt(tp, tp - dz_tty);
dh:
	if (nl[SNDH].n_type == 0)
		goto pty;
	if (kflg) {
		nl[SNDH].n_value = clear(nl[SNDH].n_value);
		nl[SDH].n_value = clear(nl[SDH].n_value);
	}
	lseek(fc, (long)nl[SNDH].n_value, 0);
	read(fc, &ndz, sizeof(ndz));
	printf("%d dh lines\n", ndz);
	lseek(fc, (long)nl[SDH].n_value, 0);
	read(fc, dz_tty, ndz * sizeof(struct tty));
	for (tp = dz_tty; tp < &dz_tty[ndz]; tp++)
		ttyprt(tp, tp - dz_tty);
pty:
	if (nl[SPTY].n_type == 0)
		goto pty;
	if (kflg) {
		nl[SPTY].n_value = clear(nl[SPTY].n_value);
	}
	printf("32 pty lines\n");
	lseek(fc, (long)nl[SPTY].n_value, 0);
	read(fc, dz_tty, 32*sizeof(struct tty));
	for (tp = dz_tty; tp < &dz_tty[32]; tp++)
		ttyprt(tp, tp - dz_tty);
}

ttyprt(atp, line)
struct tty *atp;
{
	register struct tty *tp;

	printf("%2d", line);
	tp = atp;
	switch (tp->t_line) {

/*
	case NETLDISC:
		if (tp->t_rec)
			printf("%4d%4d", 0, tp->t_inbuf);
		else
			printf("%4d%4d", tp->t_inbuf, 0);
		break;
*/

	default:
		printf("%4d", tp->t_rawq.c_cc);
		printf("%4d", tp->t_canq.c_cc);
	}
	printf("%4d", tp->t_outq.c_cc);
	printf("%8.1x", tp->t_flags);
	printf(" %8.1x", tp->t_addr);
	printf("%3d", tp->t_delct);
	printf("%4d ", tp->t_col);
	putf(tp->t_state&TS_TIMEOUT, 'T');
	putf(tp->t_state&TS_WOPEN, 'W');
	putf(tp->t_state&TS_ISOPEN, 'O');
	putf(tp->t_state&TS_CARR_ON, 'C');
	putf(tp->t_state&TS_BUSY, 'B');
	putf(tp->t_state&TS_ASLEEP, 'A');
	putf(tp->t_state&TS_XCLUDE, 'X');
	putf(tp->t_state&TS_HUPCLS, 'H');
	printf("%6d", tp->t_pgrp);
	switch (tp->t_line) {

	case NTTYDISC:
		printf(" ntty");
		break;

	case NETLDISC:
		printf(" net");
		break;
	}
	printf("\n");
}

dousr()
{
	struct user U;
	struct ucred UC;
	register i, j, *ip;

	/* This wins only if PAGSIZ > sizeof (struct user) */
	lseek(fc, ubase * NBPG, 0);
	read(fc, &U, sizeof(U));
	lseek(fc, U.u_cred, 0);
	read(fc, &UC, sizeof(UC));
	printf("pcb");
	ip = (int *)&U.u_pcb;
	while (ip < &U.u_arg[0]) {
		if ((ip - (int *)&U.u_pcb) % 4 == 0)
			printf("\t");
		printf("%x ", *ip++);
		if ((ip - (int *)&U.u_pcb) % 4 == 0)
			printf("\n");
	}
	if ((ip - (int *)&U.u_pcb) % 4 != 0)
		printf("\n");
	printf("arg\t");
	for (i=0; i<5; i++)
		printf(" %.1x", U.u_arg[i]);
	printf("\n");
	for (i=0; i<sizeof(label_t)/sizeof(int); i++) {
		if (i%5==0)
			printf("\t");
		printf("%9.1x", U.u_ssave.val[i]);
		if (i%5==4)
			printf("\n");
	}
	if (i%5)
		printf("\n");
	printf("segflg\t%d\nerror %d\n", U.u_segflg, U.u_error);
	printf("uids\t%d,%d,%d,%d\n", UC.cr_uid,UC.cr_gid,UC.cr_ruid,
	    UC.cr_rgid);
	printf("procp\t%.1x\n", U.u_procp);
	printf("ap\t%.1x\n", U.u_ap);
	printf("r_val?\t%.1x %.1x\n", U.u_r.r_val1, U.u_r.r_val2);
	printf("base, count, offset %.1x %.1x %ld\n", U.u_base,
		U.u_count, U.u_offset);
	printf("cdir rdir %.1x %.1x\n", U.u_cdir, U.u_rdir);
	printf("file");
	for (i=0; i<NOFILE; i++) {
		if (i%5==0)
			printf("\t");
		printf("%9.1x", U.u_ofile[i]);
		if (i%5==4)
			printf("\n");
	}
	printf("\n");
	printf("pofile");
	for (i=0; i<NOFILE; i++) {
		if (i%5==0)
			printf("\t");
		printf("%9.1x", U.u_pofile[i]);
		if (i%5==4)
			printf("\n");
	}
	printf("\n");
	printf("ssave");
	for (i=0; i<sizeof(label_t)/sizeof(int); i++) {
		if (i%5==0)
			printf("\t");
		printf("%9.1x", U.u_ssave.val[i]);
		if (i%5==4)
			printf("\n");
	}
	if (i%5)
		printf("\n");
	printf("sigs");
	for (i=0; i<NSIG; i++) {
		if (i%5==0)
			printf("\t");
		printf("%9.1x ", U.u_signal[i]);
		if (i%5==4)
			printf("\n");
	}
	printf("\n");
	printf("code\t%.1x\n", U.u_code);
	printf("ar0\t%.1x\n", U.u_ar0);
	printf("prof\t%X %X %X %X\n", U.u_prof.pr_base, U.u_prof.pr_size,
	    U.u_prof.pr_off, U.u_prof.pr_scale);
	printf("\neosys\t%d\n", U.u_eosys);
	printf("ttyp\t%.1x\n", U.u_ttyp);
	printf("ttyd\t%d,%d\n", major(U.u_ttyd), minor(U.u_ttyd));
	printf("exdata\t");
	ip = (int *)&U.u_exdata;
	for (i = 0; i < 8; i++)
		printf("%.1D ", *ip++);
	printf("\n");
	printf("comm %.14s\n", U.u_comm);
	printf("start\t%D\n", U.u_start);
	printf("acflag\t%D\n", U.u_acflag);
	printf("cmask\t%D\n", U.u_cmask);
	printf("sizes\t%.1x %.1x %.1x\n", U.u_tsize, U.u_dsize, U.u_ssize);
	printf("ru\t");
	ip = (int *)&U.u_ru;
	for (i = 0; i < sizeof(U.u_ru)/sizeof(int); i++)
		printf("%D ", ip[i]);
	printf("\n");
	ip = (int *)&U.u_cru;
	printf("cru\t");
	for (i = 0; i < sizeof(U.u_cru)/sizeof(int); i++)
		printf("%D ", ip[i]);
	printf("\n");
/*
	i =  U.u_stack - &U;
	while (U[++i] == 0);
	i &= ~07;
	while (i < 512) {
		printf("%x ", 0140000+2*i);
		for (j=0; j<8; j++)
			printf("%9x", U[i++]);
		printf("\n");
	}
*/
}

oatoi(s)
char *s;
{
	register v;

	v = 0;
	while (*s)
		v = (v<<3) + *s++ - '0';
	return(v);
}

dofile()
{
	int nfile;
	struct file *xfile, *afile;
	register struct file *fp;
	register nf;
	int loc;
	static char *dtypes[] = { "???", "inode", "socket" };

	nf = 0;
	nfile = getw(nl[SNFILE].n_value);
	xfile = (struct file *)calloc(nfile, sizeof (struct file));
	lseek(fc, (int)(afile = (struct file *)getw(nl[SFIL].n_value)), 0);
	read(fc, xfile, nfile * sizeof (struct file));
	for (fp=xfile; fp < &xfile[nfile]; fp++)
		if (fp->f_count)
			nf++;
	if (totflg) {
		printf("%3d/%3d files\n", nf, nfile);
		return;
	}
	printf("%d/%d open files\n", nf, nfile);
	printf("   LOC   TYPE    FLG     CNT  MSG    DATA    OFFSET\n");
	for (fp=xfile,loc=(int)afile; fp < &xfile[nfile]; fp++,loc+=sizeof(xfile[0])) {
		if (fp->f_count==0)
			continue;
		printf("%8x ", loc);
		if (fp->f_type <= DTYPE_SOCKET)
			printf("%-8.8s", dtypes[fp->f_type]);
		else
			printf("8d", fp->f_type);
		putf(fp->f_flag&FREAD, 'R');
		putf(fp->f_flag&FWRITE, 'W');
		putf(fp->f_flag&FAPPEND, 'A');
		putf(fp->f_flag&FSHLOCK, 'S');
		putf(fp->f_flag&FEXLOCK, 'X');
		putf(fp->f_flag&FASYNC, 'I');
		printf("  %3d", mask(fp->f_count));
		printf("  %3d", mask(fp->f_msgcount));
		printf("  %8.1x", fp->f_data);
		if (fp->f_offset < 0)
			printf("  %x\n", fp->f_offset);
		else
			printf("  %ld\n", fp->f_offset);
	}
}

#ifdef sun
union {
	struct	user user;
	char	upages[UPAGES][NBPG];
} user;
int	swap = -1;
char	*swapf = "/dev/drum";
int	mem = -1;
char	*memf = "/dev/mem";

getu(p)
	struct proc *p;
{
	struct pte *pteaddr, apte;
	struct pte arguutl[UPAGES+CLSIZE];
	register int i;
	int ncl, size;

	if (!kflg && swap < 0) {
		swap = open(swapf, 0);
		if (swap < 0) {
			perror(swapf);
			exit(1);
		}
	}
	if (!kflg && mem < 0) {
		mem = open(memf, 0);
		if (mem < 0) {
			perror(memf);
			exit(1);
		}
	}
	size = roundup(sizeof (struct user), DEV_BSIZE);
	if ((p->p_flag & SLOAD) == 0) {
		if (swap < 0)
			return (0);
		(void) lseek(swap, (long)dtob(p->p_swaddr), 0);
		if (read(swap, (char *)&user.user, size) != size) {
			printf("pstat: cant read u for pid %d from %s\n",
			    p->p_pid, swapf);
			return (0);
		}
		return (1);
	}
	if (kflg)
		p->p_p0br = (struct pte *)clear(p->p_p0br);
	pteaddr = &Usrptma[btokmx(p->p_p0br) + p->p_szpt - 1];
	if (kflg)
		pteaddr = (struct pte *)clear(pteaddr);
	lseek(fc, (long)pteaddr, 0);
	if (read(fc, (char *)&apte, sizeof(apte)) != sizeof(apte)) {
		printf("pstat: cant read indir pte to get u for pid %d from %s\n",
		    p->p_pid, swapf);
		return (0);
	}
	lseek(mem,
	    (long)ctob(apte.pg_pfnum+1) - (UPAGES+CLSIZE) * sizeof (struct pte),
		0);
	if (read(mem, (char *)arguutl, sizeof(arguutl)) != sizeof(arguutl)) {
		printf("pstat: cant read page table for u of pid %d from %s\n",
		    p->p_pid, memf);
		return (0);
	}
	ncl = (size + NBPG*CLSIZE - 1) / (NBPG*CLSIZE);
	while (--ncl >= 0) {
		i = ncl * CLSIZE;
		lseek(mem, (long)ctob(arguutl[CLSIZE+i].pg_pfnum), 0);
		if (read(mem, user.upages[i], CLSIZE*NBPG) != CLSIZE*NBPG) {
			printf("pstat: cant read page %d of u of pid %d from %s\n",
			    arguutl[CLSIZE+i].pg_pfnum, p->p_pid, memf);
			return(0);
		}
	}
	return (1);
}
#endif

int dmmin, dmmax, nswdev;

doswap()
{
	struct proc *proc;
	int nproc;
	struct text *xtext;
	int ntext;
	struct map *swapmap;
	int nswapmap;
	register struct proc *pp;
	int nswap, used, tused, free, waste;
	int db, sb;
	register struct mapent *me;
	register struct text *xp;
	int i, j, block;
	int dmap[NDMAP];

	nproc = getw(nl[SNPROC].n_value);
	proc = (struct proc *)calloc(nproc, sizeof (struct proc));
	lseek(fc, getw(nl[SPROC].n_value), 0);
	read(fc, proc, nproc * sizeof (struct proc));
	nswapmap = getw(nl[SNSWAPMAP].n_value);
	swapmap = (struct map *)calloc(nswapmap, sizeof (struct map));
	lseek(fc, getw(nl[SWAPMAP].n_value), 0);
	read(fc, swapmap, nswapmap * sizeof (struct map));
	swapmap->m_name = "swap";
	nswap = getw(nl[SNSWAP].n_value);
	dmmin = getw(nl[SDMMIN].n_value);
	dmmax = getw(nl[SDMMAX].n_value);
	nswdev = getw(nl[SNSWDEV].n_value);
	free = 0;
	for (me = (struct mapent *)(swapmap+1);
	    me < (struct mapent *)&swapmap[nswapmap]; me++)
		free += me->m_size;
	ntext = getw(nl[SNTEXT].n_value);
	xtext = (struct text *)calloc(ntext, sizeof (struct text));
	lseek(fc, getw(nl[STEXT].n_value), 0);
	read(fc, xtext, ntext * sizeof (struct text));
	tused = 0;
	for (xp = xtext; xp < &xtext[ntext]; xp++)
		if (xp->x_vptr!=NULL)
			tused += xdsize(xp);
	used = tused;
	waste = 0;
	for (pp = proc; pp < &proc[nproc]; pp++) {
		if (pp->p_stat == 0 || pp->p_stat == SZOMB)
			continue;
		if (pp->p_flag & SSYS)
			continue;
		db = ctod(pp->p_dsize);
		sb = ctod(pp->p_ssize);
		waste -= db + sb;
		db = up(db);
		sb = up(sb);
		used += db + sb;
		waste += db + sb;
		if ((pp->p_flag&SLOAD) == 0)
			used += vusize(pp);
#ifdef sun
		if (getu(pp) && user.user.u_hole.uh_last) {
			used -= ctod(user.user.u_hole.uh_last -
			    user.user.u_hole.uh_first + 1);
		}
#endif
	}
	/* a DMMAX/2 block goes to argmap */
	if (totflg) {
		printf("%3d/%3d 00k swap\n", used/100, (used+free)/100);
		return;
	}
	printf("%d used (%d text), %d free, %d wasted, %d missing\n",
	    used, tused, free, waste, (nswap - dmmax/2 - (used + free)));
	block = dmmin;
	for (i=0, j=0; i<NDMAP; i++) {
		dmap[i] = rmalloc(swapmap, block);
		if (dmap[i] == 0)
			break;
		j += block;
		if (block < dmmax)
			block *= 2;
	}
	printf("max process allocable = %dK\n", j*DEV_BSIZE/1024);
	block = dmmin;
	for (j=0; j<i; j++) {
		rmfree(swapmap, block, dmap[j]);
		if (block < dmmax)
			block *= 2;
	}
	printf("avail: ");
	for (i = dmmax; i >= dmmin; i /= 2) {
		j = 0;
		while (rmalloc(swapmap, i) != 0)
			j++;
		if (j) printf("%d*%d ", j, i);
	}
	printf("\n");
}

up(size)
	register int size;
{
	register int i, block;

	i = 0;
	block = dmmin;
	while (i < size) {
		i += block;
		if (block < dmmax)
			block *= 2;
	}
	return (i);
}

vusize(p)
	struct proc *p;
{
	register int tsz = p->p_tsize / NPTEPG;

	return (ctod(clrnd(UPAGES +
	    clrnd(ctopt(p->p_tsize+p->p_dsize+p->p_ssize+UPAGES)) - tsz)));
}

xdsize(xp)
	struct text *xp;
{

	if (xp->x_flag & XPAGV)
		return (ctod(clrnd(xp->x_size + ctopt(xp->x_size))));
	return (ctod(xp->x_size));
}

/*
 * Allocate 'size' units from the given
 * map. Return the base of the allocated space.
 * In a map, the addresses are increasing and the
 * list is terminated by a 0 size.
 *
 * Algorithm is first-fit.
 *
 * This routine knows about the interleaving of the swapmap
 * and handles that.
 */
long
rmalloc(mp, size)
	register struct map *mp;
	long size;
{
	register struct mapent *ep = (struct mapent *)(mp+1);
	register int addr;
	register struct mapent *bp;
	swblk_t first, rest;

	if (size <= 0 || size > dmmax)
		return (0);
	/*
	 * Search for a piece of the resource map which has enough
	 * free space to accomodate the request.
	 */
	for (bp = ep; bp->m_size; bp++) {
		if (bp->m_size >= size) {
			/*
			 * If allocating from swapmap,
			 * then have to respect interleaving
			 * boundaries.
			 */
			if (nswdev > 1 &&
			    (first = dmmax - bp->m_addr%dmmax) < bp->m_size) {
				if (bp->m_size - first < size)
					continue;
				addr = bp->m_addr + first;
				rest = bp->m_size - first - size;
				bp->m_size = first;
				if (rest)
					rmfree(mp, rest, addr+size);
				return (addr);
			}
			/*
			 * Allocate from the map.
			 * If there is no space left of the piece
			 * we allocated from, move the rest of
			 * the pieces to the left.
			 */
			addr = bp->m_addr;
			bp->m_addr += size;
			if ((bp->m_size -= size) == 0) {
				do {
					bp++;
					(bp-1)->m_addr = bp->m_addr;
				} while ((bp-1)->m_size = bp->m_size);
			}
			if (addr % CLSIZE)
				return (0);
			return (addr);
		}
	}
	return (0);
}

/*
 * Free the previously allocated space at addr
 * of size units into the specified map.
 * Sort addr into map and combine on
 * one or both ends if possible.
 */
rmfree(mp, size, addr)
	struct map *mp;
	long size, addr;
{
	struct mapent *firstbp;
	register struct mapent *bp;
	register int t;

	/*
	 * Both address and size must be
	 * positive, or the protocol has broken down.
	 */
	if (addr <= 0 || size <= 0)
		goto badrmfree;
	/*
	 * Locate the piece of the map which starts after the
	 * returned space (or the end of the map).
	 */
	firstbp = bp = (struct mapent *)(mp + 1);
	for (; bp->m_addr <= addr && bp->m_size != 0; bp++)
		continue;
	/*
	 * If the piece on the left abuts us,
	 * then we should combine with it.
	 */
	if (bp > firstbp && (bp-1)->m_addr+(bp-1)->m_size >= addr) {
		/*
		 * Check no overlap (internal error).
		 */
		if ((bp-1)->m_addr+(bp-1)->m_size > addr)
			goto badrmfree;
		/*
		 * Add into piece on the left by increasing its size.
		 */
		(bp-1)->m_size += size;
		/*
		 * If the combined piece abuts the piece on
		 * the right now, compress it in also,
		 * by shifting the remaining pieces of the map over.
		 */
		if (bp->m_addr && addr+size >= bp->m_addr) {
			if (addr+size > bp->m_addr)
				goto badrmfree;
			(bp-1)->m_size += bp->m_size;
			while (bp->m_size) {
				bp++;
				(bp-1)->m_addr = bp->m_addr;
				(bp-1)->m_size = bp->m_size;
			}
		}
		goto done;
	}
	/*
	 * Don't abut on the left, check for abutting on
	 * the right.
	 */
	if (addr+size >= bp->m_addr && bp->m_size) {
		if (addr+size > bp->m_addr)
			goto badrmfree;
		bp->m_addr -= size;
		bp->m_size += size;
		goto done;
	}
	/*
	 * Don't abut at all.  Make a new entry
	 * and check for map overflow.
	 */
	do {
		t = bp->m_addr;
		bp->m_addr = addr;
		addr = t;
		t = bp->m_size;
		bp->m_size = size;
		bp++;
	} while (size = t);
	/*
	 * Segment at bp is to be the delimiter;
	 * If there is not room for it 
	 * then the table is too full
	 * and we must discard something.
	 */
	if (bp+1 > mp->m_limit) {
		/*
		 * Back bp up to last available segment.
		 * which contains a segment already and must
		 * be made into the delimiter.
		 * Discard second to last entry,
		 * since it is presumably smaller than the last
		 * and move the last entry back one.
		 */
		bp--;
		printf("%s: rmap ovflo, lost [%d,%d)\n", mp->m_name,
		    (bp-1)->m_addr, (bp-1)->m_addr+(bp-1)->m_size);
		bp[-1] = bp[0];
		bp[0].m_size = bp[0].m_addr = 0;
	}
done:
	return;
badrmfree:
	printf("bad rmfree\n");
}
