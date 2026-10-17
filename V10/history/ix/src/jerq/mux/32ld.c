/*
 * 32ld as a subroutine in mux, just to do initial raw download of muxterm
 */
#include <signal.h>
#include "aouthdr.h"
#include "filehdr.h"
#include "scnhdr.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/label.h>

extern int	untrusted;
extern int	booted;

#define	MAXPKTSIZE	128		/* Efficient size for system */
#define NPBUFS	2

#define NSECTS  12

#define MPX_VER 0x5620
#define FBOMAGIC 0560

struct filehdr fileheader;
struct aouthdr aoutheader;
struct scnhdr secthdrs[NSECTS];

#define ioctl_t struct sgttyb *
int	obj;            /* File descriptor for object file */
int	packsiz = 120;
int	open();
void	exit();
long	lseek();
char	*memcpy();
unsigned	alarm();


void    Precv();
void	Write();
void precv32();
void apsend();



_32ld(s, speed)
char	*s;
{
	char c = 0;
	struct stat statbuf;
	int	i;
	struct label lab;

	packsiz = min(120, speed/2);
	dmd_ai(); 			/* right rom ? */

	obj = open(s,  0);
	if (obj < 0)
		quit( "cannot open download");
	fstat(obj, &statbuf);
	if (fgetflab(obj, &lab) == -1)
		quit( "cannot get label download");
	if (lab.lb_t == 0 && lab.lb_u == 0)
		untrusted++;

	(void)Read ((char * )&fileheader, sizeof(struct filehdr ));
	if (fileheader.f_magic != FBOMAGIC)        /* FBOMAGIC is 0560 */
		quit( "download not a MAC-32 a.out");
	(void)Read ((char * )&aoutheader, (int)fileheader.f_opthdr);

	if (fileheader.f_nscns > NSECTS)
		quit( "32ld: too many sections");
	if ((aoutheader.vstamp == MPX_VER) )  /* MPX_VER is 0x5620 */
		quit( "download compiled for mpx");

	write(1, "\033[0;0v", 6);
	while (c != 'a' && c != 1)
		(void)read(0, &c, 1); /* wait for terminal to be ready */
	booted++;

	ptimeout32();
	for (i = 0; i < fileheader.f_nscns; ++i)
		(void)Read ((char * )&secthdrs[i], sizeof(struct scnhdr ));

	for (i=0; i<fileheader.f_nscns; i++) {
		if (secthdrs[i].s_scnptr > 0)  {
			if ((secthdrs[i].s_flags & STYP_NOLOAD) || 
			    (secthdrs[i].s_flags & STYP_DSECT))
				continue;
			(void)lseek(obj, (long)secthdrs[i].s_scnptr, 0);
			sendseg(secthdrs[i].s_paddr, secthdrs[i].s_paddr + secthdrs[i].s_size);
		}
	}
	close(obj);
	Precv(NPBUFS);
	apsend(aoutheader.entry, (char *)0, 0);
	Precv(NPBUFS);
	sleep(2);
}

#define TERM_1_0 "\033[?8;7;1c"
#define TERMB_1_0 "\033[?8;7;2c"
#define TERM_DMD "\033[?8;"
#define TERMIDSIZE 9
char	termid[TERMIDSIZE+1];

dmd_ai() {
	int i;

	write(1, "\033[c", 3);
	for (i = 0; i < TERMIDSIZE; i++)
		if (read(0, &termid[i], 1) != 1)
			quit( "read error");
	if ((strcmp(termid, TERM_1_0) == 0) || 
	(strcmp(termid, TERMB_1_0) == 0))
		quit( "Error: Firmware older than 1.1");
	if (strncmp(termid, TERM_DMD, strlen(TERM_DMD)) != 0)
		quit( "Error: 32ld needs a 5620 terminal");
}


int
Read(a, n)
char	*a;
{
	register i;
	i = read(obj, a, n);
	if (i < 0)
		quit( "read error on muxterm");
	return(i);
}


void
Write(a, n)
char	*a;
{
	if (write(1, a, n) != n)
		quit( "write error to jerq");
}


sendseg(strloc, endloc)
long	strloc;
long	endloc;
{
	char	buf[MAXPKTSIZE];
	register n;

	while ((n = Read(&buf[0], min(packsiz, (int)(endloc - strloc)))) > 0) {
		Precv(1);
		apsend(strloc, buf, n);
		strloc += n;
	}
}

#define	PTYP	0xc0		/* Masked with Seq to guarantee non-zero */
#define	ACKON	0x80		/* Ack expected for this packet */
#define	SEQMASK	0x3f
#define SEQMOD	0x40		/* SEQMASK+1 */

#define	WAIT	1
#define	OK	2

#define MAXRETRIES      10


struct Packet {
	char	packet[MAXPKTSIZE];
	short	size;
	short	timo;
	short	state;
};

typedef struct Packet *	Pkp_t;

struct Packet packets[NPBUFS];
char	xseq;
char	freepkts = NPBUFS;
char	nulls[MAXPKTSIZE-2];
short	xtimo = 3;
int	retries;

void
Precv(need)
{
	char	c;

	retries = 0;
	while (freepkts < need) {
		(void)alarm(3);               /* sleep at least 2 seconds */
		if (read(0, &c, 1) == 1) {
			(void)alarm(0);
			precv32(c);
		} else if (errno != EINTR )
			quit( "read error");
		else if (++retries >= MAXRETRIES)
			quit( "load protocol failed");
	}
}




void
precv32(c)
char	c;
{
	register Pkp_t	pkp;
	register char	pseq, seq = c &SEQMASK;

	if ((c & PTYP) != ACKON) {
		return;
	}
	for (pkp = packets; pkp < &packets[NPBUFS]; pkp++)
		if (pkp->state == WAIT) {	
			pseq = (pkp->packet[0]) & SEQMASK;
			if (seq == pseq) {
				pkp->state = OK;
				freepkts++;
			} else if ((seq > pseq && seq < pseq + NPBUFS)
			     ||  (seq < pseq && seq + SEQMOD < pseq + NPBUFS)) {
				pkp->state = OK;
				freepkts++;
			}	
		}
}


void
apsend(x, bufp, count)
long	x;
char	*bufp;
register int	count;
{
	register Pkp_t	pkp;
	register int	i;
	char	*s;

	for (pkp = (Pkp_t)0, i = 0; i < NPBUFS; i++)
		if (packets[i].state != WAIT) {
			if (pkp == (Pkp_t)0) {
				pkp = &packets[i];
				pkp->state = WAIT;
				pkp->timo = xtimo;
				freepkts--;
				break;
			}
		}
	if (pkp == (Pkp_t)0)
		return;
	s = &pkp->packet[0];
	*s++ = ACKON | ((xseq++) & SEQMASK);
	*s++ = count + 4;
	*s++ = x >> 24;
	*s++ = x >> 16;
	*s++ = x >> 8;
	*s++ = x;
	if (bufp)
		(void)memcpy(s, bufp, count);
	count += 6;
	crc((unsigned char * )pkp->packet, count);
	count += 2;
	pkp->size = count;
	Write(pkp->packet, count);
}


ptimeout32()
{	
	register Pkp_t	pkp;

	(void)signal(SIGALRM, SIG_IGN);
	for (pkp = packets; pkp < &packets[NPBUFS]; pkp++)
		if (pkp->state == WAIT && --pkp->timo <= 0) {
			pkp->timo = xtimo;
			Write(nulls, pkp->size - 2);
			Write((char * )pkp->packet, pkp->size);
		}
	(void)signal(SIGALRM, ptimeout32);
}
/*
 *	Packet format:
 *
 *	0x80|Seq, Size, 4 bytes of address, n >= 0 data bytes , 2 bytes crc
 *
 *	where 4 <= Size <= (4+MAXPKTDSIZE).
 *
 *	If Size == 4 then the address is the start address.
 *	If Size > 4 then load data contiguously at the address.
 *
 *	Acknowledgements consist of the first byte
 *	from each correctly received packet (Seq)
 *	acks apply to all packets sent, up to and including the one acked.
 *	Timeouts cause retransmission of all unacknowledged packets
 */
