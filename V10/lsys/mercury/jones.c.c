/*
 * config data
 */

#include "sys/param.h"
#include "sys/conf.h"
#include "sys/vtimes.h"
#include "sys/proc.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/text.h"
#include "sys/callout.h"
#include "sys/buf.h"
#include "sys/map.h"
#include "sys/stream.h"
#include "sys/mba.h"
#include "sys/mbaddr.h"
#include "sys/nxaddr.h"
#include "sys/nexus.h"
#include "sys/ubaddr.h"
#include "sys/uba.h"
#include "sys/uda.h"
#include "sys/inet/in.h"
#include "sys/inet/ip_var.h"
#include "sys/inet/udp.h"
#include "sys/inet/tcp.h"
#include "sys/inet/tcp_timer.h"
#include "sys/inet/tcp_var.h"
#include "sys/subaddr.h"
#include "sys/dz.h"
#include "sys/kmc.h"
#include "sys/kdi.h"
#include "sys/mscp.h"
#include "sys/ni1010a.h"
#include "sys/udaioc.h"
#include "sys/ra.h"
#include "sys/tu78.h"
#include "sys/hp.h"
#include "sys/bad144.h"
#include "sys/ttyio.h"
#include "sys/ttyld.h"
#include "sys/bufld.h"
#include "sys/mesg.h"
#include "sys/nttyio.h"
#include "sys/nttyld.h"
#include "sys/mount.h"
extern struct bdevsw hpbdev;
extern struct bdevsw swbdev;
extern struct bdevsw rabdev;
extern struct bdevsw tu78bdev;
int nblkdev = 9;
extern struct cdevsw cncdev;
extern struct cdevsw dzcdev;
extern struct cdevsw mmcdev;
extern struct cdevsw hpcdev;
extern struct cdevsw swcdev;
extern struct cdevsw tu78cdev;
extern struct cdevsw kmccdev;
extern struct cdevsw racdev;
extern struct cdevsw kdicdev;
extern struct cdevsw fdcdev;
extern struct cdevsw ipcdev;
extern struct cdevsw tcpcdev;
extern struct cdevsw ilcdev;
extern struct cdevsw udpcdev;
int nchrdev = 51;
extern struct fstypsw fsfs;
extern struct fstypsw nafs;
extern struct fstypsw prfs;
extern struct fstypsw msfs;
extern struct fstypsw nbfs;
extern struct fstypsw erfs;
extern struct fstypsw pipfs;
int nfstyp = 7;
extern struct streamtab ttystream;
extern struct streamtab msgstream;
extern struct streamtab nttystream;
extern struct streamtab bufldstream;
extern struct streamtab rmsgstream;
extern struct streamtab ipstream;
extern struct streamtab tcpstream;
extern struct streamtab udpstream;
extern struct streamtab connstream;
extern struct streamtab xpstream;
int nstreamtab = 20;

struct bdevsw *bdevsw[] = {
	&hpbdev,	/* 0 */
	NULL,
	NULL,
	NULL,
	&swbdev,	/* 4 */
	NULL,
	NULL,
	&rabdev,	/* 7 */
	&tu78bdev,	/* 8 */
};
struct cdevsw *cdevsw[] = {
	&cncdev,	/* 0 */
	&dzcdev,	/* 1 */
	NULL,
	&mmcdev,	/* 3 */
	&hpcdev,	/* 4 */
	NULL,
	NULL,
	&swcdev,	/* 7 */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&tu78cdev,	/* 22 */
	NULL,
	NULL,
	NULL,
	&kmccdev,	/* 26 */
	NULL,
	&racdev,	/* 28 */
	NULL,
	NULL,
	&kdicdev,	/* 31 */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&fdcdev,	/* 40 */
	NULL,
	&ipcdev,	/* 42 */
	&tcpcdev,	/* 43 */
	&ilcdev,	/* 44 */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&udpcdev,	/* 50 */
};
struct fstypsw *fstypsw[] = {
	&fsfs,	/* 0 */
	&nafs,	/* 1 */
	&prfs,	/* 2 */
	&msfs,	/* 3 */
	&nbfs,	/* 4 */
	&erfs,	/* 5 */
	&pipfs,	/* 6 */
};
struct streamtab *streamtab[] = {
	&ttystream,	/* 0 */
	NULL,
	NULL,
	NULL,
	&msgstream,	/* 4 */
	NULL,
	&nttystream,	/* 6 */
	&bufldstream,	/* 7 */
	NULL,
	&rmsgstream,	/* 9 */
	&ipstream,	/* 10 */
	&tcpstream,	/* 11 */
	NULL,
	NULL,
	&udpstream,	/* 14 */
	NULL,
	NULL,
	NULL,
	&connstream,	/* 18 */
	&xpstream,	/* 19 */
};

struct nextab nextab[] = {
	0, 0,
	0, 4,
	0, 5,
	0, 6,
	0, 8,
	-1
};
struct nxaddr mcraddr[] = {
	{0, 0, 0x0},
};
int mcrcnt = 1;
caddr_t mcrregs[1];
time_t mcrtime[1];
int proccnt = 300;
struct proc proc[300];
int filecnt = 500;
struct file file[500];
int inodecnt = 700;
struct inode inode[700];
int calloutcnt = 50;
struct callout callout[50];
int textcnt = 60;
struct text text[60];
int argcnt = 16;
struct map argmap[16];
int swmapcnt = 200;
struct map swapmap[200];
int kernelcnt = 100;
struct map kernelmap[100];
int swbufcnt = 50;
struct buf swapbuf[50];
struct swapinfo swapinfo[50];
int bufhcnt = 63;
struct bufhd bufhash[63];
int dstflag = 1;
int timezone = 300;
int maxtsize = 12256;
int maxdsize = 24544;
int maxssize = 24544;
struct nxaddr mbaaddr[] = {
	{1, 0, 0x0},
	{2, 0, 0x0},
	{3, 0, 0x0},
};
int mbacnt = 3;
struct mba mba[3];
extern hp0int();
extern hp0int();
extern hp0int();
extern hp0int();
extern hp0int();
extern hp0int();
extern hp0int();
extern hp0int();
extern tm780int();
extern hp0int();
extern hp0int();
extern hp0int();
extern hp0int();
char mbaid[][8] = {
 0, 1, 2, 3, 8, 9, 10, 11,
 0, 011, 012, 013, 014, 015, 016, 017,
 4, 5, 6, 7, 024, 025, 026, 027,
};
int mbastray();
int (*mbavec[][8])() = {
 hp0int, hp0int, hp0int, hp0int, hp0int, hp0int, hp0int, hp0int,
 tm780int, mbastray, mbastray, mbastray, mbastray, mbastray, mbastray, mbastray,
 hp0int, hp0int, hp0int, hp0int, mbastray, mbastray, mbastray, mbastray,
};
struct mbaddr hpaddr[] = {
	{0, 0},
	{1, 0},
	{2, 0},
	{3, 0},
	{0, 2},
	{1, 2},
	{2, 2},
	{3, 2},
	{4, 0},
	{5, 0},
	{6, 0},
	{7, 0},
};
int hpcnt = 12;
struct hpdisk hpdisk[12];
struct buf hpbuf[12];
struct buf hpbadbuf[12];
struct bad144 hpbad[12];
struct mbaddr tm78addr[] = {
	{0, 1},
};
int tm78cnt = 1;
struct tm78 tm78[1];
struct subaddr tu78addr[] = {
	{0, 0},
};
int tu78cnt = 1;
struct tu78 tu78[1];
struct buf ctu78buf[1];
struct buf rtu78buf[1];
struct nxaddr ubaaddr[] = {
	{4, 0, 0x200},
};
int ubacnt = 1;
struct uba uba[1];
struct ubaddr udaddr[] = {
	{0772150, 0254, 0},
	{0772160, 0260, 0},
};
int udcnt = 2;
struct ud ud[2];
extern struct msportsw udport;
int nmsport = 1;
struct msportsw *msportsw[] = {
	&udport,	/* 0 */
};
struct msaddr raaddr[] = {
	{0, 0, 0},
	{0, 0, 1},
	{0, 0, 2},
	{0, 0, 3},
	{1, 0, 0},
	{1, 0, 1},
	{1, 0, 2},
	{1, 0, 3},
};
int racnt = 8;
struct radisk radisk[8];
struct buf rabuf[8];
struct ubaddr dzaddr[] = {
	{0760100, 0300, 0},
};
int dzcnt = 8;
struct dz dz[8];
struct ubaddr kmcaddr[] = {
	{0760300, 0600, 0},
};
int kmccnt = 1;
struct kmc kmc[1];
struct ubaddr iladdr[] = {
	{0764000, 0340, 0},
};
int ilcnt = 1;
struct il il[1];
int kdicnt = 1;
struct kdikmc kdikmc[1];
struct kmcdk k[1];
int cncnt = 0;
int ipcnt = 4;
struct ipif ipif[4];
struct ipif *ipifsort[4];
int udpcnt = 16;
struct udp udpconn[16];
int tcpcnt = 32;
struct tcpcb tcpcb[32];
int arpcnt = 64;
struct ip_arp ip_arps[64];
int ttycnt = 48;
struct ttyld ttyld[48];
int nttycnt = 32;
struct nttyld ntty[32];
int msgcnt = 64;
struct imesg mesg[64];
int xpcnt = 0;
int bufldcnt = 4;
struct bufld bufld[4];
int fscnt = 40;
struct mount fsmtab[40];
int blkbcnt = 20;
struct buf *cblkbuf[20];
int queuecnt = 2000;
struct queue queue[2000];
int streamcnt = 512;
struct stdata streams[512];
int blkcnt = 1000;
struct block cblock[1000];
int rootfstyp = 0;
dev_t rootdev = makedev(7, 64);
dev_t swapdev = makedev(4, 0);
struct swdevt swdevt[] = {
	{makedev(0, 1), 21120L},
	{makedev(0, 9), 21120L},
};
int nswdevt = 2;
extern int hpdump();
int (*dumprout)() = hpdump;
int dumpunit = 1;
long dumplow = 10560;
long dumpsize = 21120;
