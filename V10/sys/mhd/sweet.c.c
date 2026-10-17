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
#include "sys/kmc.h"
#include "sys/kdi.h"
#include "sys/mscp.h"
#include "sys/scsi.h"
#include "sys/ni1010a.h"
#include "sys/udaioc.h"
#include "sys/ra.h"
#include "sys/ttyio.h"
#include "sys/ttyld.h"
#include "sys/bufld.h"
#include "sys/mesg.h"
#include "sys/mount.h"
extern struct bdevsw swbdev;
extern struct bdevsw rabdev;
int nblkdev = 8;
extern struct cdevsw cncdev;
extern struct cdevsw mmcdev;
extern struct cdevsw swcdev;
extern struct cdevsw scsicdev;
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
extern struct fstypsw prfs;
extern struct fstypsw msfs;
extern struct fstypsw nbfs;
extern struct fstypsw erfs;
extern struct fstypsw pipfs;
int nfstyp = 7;
extern struct streamtab ttystream;
extern struct streamtab msgstream;
extern struct streamtab bufldstream;
extern struct streamtab rmsgstream;
extern struct streamtab ipstream;
extern struct streamtab tcpstream;
extern struct streamtab udpstream;
extern struct streamtab connstream;
extern struct streamtab xpstream;
int nstreamtab = 20;

struct bdevsw *bdevsw[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	&swbdev,	/* 4 */
	NULL,
	NULL,
	&rabdev,	/* 7 */
};
struct cdevsw *cdevsw[] = {
	&cncdev,	/* 0 */
	NULL,
	NULL,
	&mmcdev,	/* 3 */
	NULL,
	NULL,
	NULL,
	&swcdev,	/* 7 */
	NULL,
	NULL,
	NULL,
	&scsicdev,	/* 11 */
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
	NULL,
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
	NULL,
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
int queuecnt = 1024;
struct queue queue[1024];
int streamcnt = 256;
struct stdata streams[256];
int blkcnt = 600;
struct block cblock[600];
int blkbcnt = 10;
struct buf *cblkbuf[10];
int dstflag = 1;
int timezone = 300;
int maxtsize = 12256;
int maxdsize = 24544;
int maxssize = 24544;
struct nxaddr ubaaddr[] = {
	{1, 0, 0x200},
};
int ubacnt = 1;
struct uba uba[1];
struct ubaddr udaddr[] = {
	{0772150, 0254, 0},
	{0760334, 0260, 0},
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
};
int racnt = 6;
struct radisk radisk[6];
struct buf rabuf[6];
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
struct ubaddr scsiaddr[] = {
	{0772154, 0700, 0},
};
int scsicnt = 1;
struct scsi scsi[1];
int kdicnt = 1;
struct kdikmc kdikmc[1];
struct kmcdk k[1];
int cncnt = 0;
int ipcnt = 4;
struct ipif ipif[4];
struct ipif *ipifsort[4];
int udpcnt = 16;
struct udp udpconn[16];
int tcpcnt = 64;
struct tcpcb tcpcb[64];
int arpcnt = 128;
struct ip_arp ip_arps[128];
int ttycnt = 64;
struct ttyld ttyld[64];
int msgcnt = 64;
struct imesg mesg[64];
int xpcnt = 0;
int bufldcnt = 4;
struct bufld bufld[4];
int fscnt = 30;
struct mount fsmtab[30];
int rootfstyp = 0;
dev_t rootdev = makedev(7, 64);
dev_t swapdev = makedev(4, 0);
struct swdevt swdevt[] = {
	{makedev(7, 1), 20480L},
	{makedev(7, 9), 20480L},
	{makedev(7, 17), 20480L},
	{makedev(7, 25), 20480L},
};
int nswdevt = 4;
extern int uddump();
int (*dumprout)() = uddump;
int dumpunit = 0;
long dumplow = 10240;
long dumpsize = 20480;
