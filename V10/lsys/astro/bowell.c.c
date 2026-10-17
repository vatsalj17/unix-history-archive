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
#include "sys/nexus.h"
#include "sys/ubaddr.h"
#include "sys/uba.h"
#include "sys/biaddr.h"
#include "sys/uda.h"
#include "sys/bda.h"
#include "sys/inet/in.h"
#include "sys/inet/ip_var.h"
#include "sys/inet/udp.h"
#include "sys/inet/tcp.h"
#include "sys/inet/tcp_timer.h"
#include "sys/inet/tcp_var.h"
#include "sys/dz.h"
#include "sys/dk.h"
#include "sys/kmc.h"
#include "sys/kdi.h"
#include "sys/kb.h"
#include "sys/mscp.h"
#include "sys/udaioc.h"
#include "sys/ra.h"
#include "sys/ta.h"
#include "sys/bvp.h"
#include "sys/debna.h"
#include "sys/ttyio.h"
#include "sys/ttyld.h"
#include "sys/bufld.h"
#include "sys/mesg.h"
#include "sys/dkp.h"
#include "sys/mount.h"
extern struct bdevsw cbsbdev;
extern struct bdevsw swbdev;
extern struct bdevsw rabdev;
extern struct bdevsw tabdev;
int nblkdev = 11;
extern struct cdevsw cncdev;
extern struct cdevsw dzcdev;
extern struct cdevsw mmcdev;
extern struct cdevsw clkcdev;
extern struct cdevsw swcdev;
extern struct cdevsw kbcdev;
extern struct cdevsw cbscdev;
extern struct cdevsw bnacdev;
extern struct cdevsw dkcdev;
extern struct cdevsw kmccdev;
extern struct cdevsw racdev;
extern struct cdevsw kdicdev;
extern struct cdevsw fdcdev;
extern struct cdevsw ipcdev;
extern struct cdevsw tcpcdev;
extern struct cdevsw udpcdev;
extern struct cdevsw tacdev;
extern struct cdevsw curecdev;
extern struct cdevsw rcurecdev;
int nchrdev = 62;
extern struct fstypsw fsfs;
extern struct fstypsw nafs;
extern struct fstypsw prfs;
extern struct fstypsw msfs;
extern struct fstypsw nbfs;
extern struct fstypsw erfs;
extern struct fstypsw pipfs;
int nfstyp = 7;
extern struct streamtab ttystream;
extern struct streamtab cdkpstream;
extern struct streamtab rdkstream;
extern struct streamtab msgstream;
extern struct streamtab dkpstream;
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
	&cbsbdev,	/* 3 */
	&swbdev,	/* 4 */
	NULL,
	NULL,
	&rabdev,	/* 7 */
	NULL,
	NULL,
	&tabdev,	/* 10 */
};
struct cdevsw *cdevsw[] = {
	&cncdev,	/* 0 */
	&dzcdev,	/* 1 */
	NULL,
	&mmcdev,	/* 3 */
	NULL,
	NULL,
	&clkcdev,	/* 6 */
	&swcdev,	/* 7 */
	NULL,
	NULL,
	&kbcdev,	/* 10 */
	NULL,
	&cbscdev,	/* 12 */
	NULL,
	&bnacdev,	/* 14 */
	NULL,
	NULL,
	&dkcdev,	/* 17 */
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
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&udpcdev,	/* 50 */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&tacdev,	/* 59 */
	&curecdev,	/* 60 */
	&rcurecdev,	/* 61 */
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
	&cdkpstream,	/* 1 */
	&rdkstream,	/* 2 */
	NULL,
	&msgstream,	/* 4 */
	&dkpstream,	/* 5 */
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
int proccnt = 600;
struct proc proc[600];
int filecnt = 1500;
struct file file[1500];
int inodecnt = 1700;
struct inode inode[1700];
int calloutcnt = 50;
struct callout callout[50];
int textcnt = 120;
struct text text[120];
int argcnt = 16;
struct map argmap[16];
int swmapcnt = 600;
struct map swapmap[600];
int kernelcnt = 500;
struct map kernelmap[500];
int swbufcnt = 50;
struct buf swapbuf[50];
struct swapinfo swapinfo[50];
int bufhcnt = 599;
struct bufhd bufhash[599];
int queuecnt = 3000;
struct queue queue[3000];
int streamcnt = 800;
struct stdata streams[800];
int blkcnt = 1600;
struct block cblock[1600];
int blkbcnt = 20;
struct buf *cblkbuf[20];
int dstflag = 1;
int timezone = 300;
int maxtsize = 12256;
int maxdsize = 204800;
int maxssize = 204800;

struct nextab nextab[] = {
	2, 2,
	2, 4,
	2, 0,
	2, 6,
	2, 7,
	-1
};
struct biaddr biaaddr[] = {
	{-1},
	{-1},
	{0, 0x4, 0x400, 2},
};
int biacnt = 3;
int bianode[3];
struct biaddr bdaddr[] = {
	{1, 0x10, 0x0, 2},
};
int bdcnt = 1;
struct bd bd[1];
struct biaddr ubaaddr[] = {
	{2, 0x20, 0x200, 2},
	{3, 0x30, 0x800, 2},
};
int ubacnt = 2;
struct uba uba[2];
struct ubaddr udaddr[] = {
	{0774500, 0300, 1},
	{0772150, 0254, 0},
	{0760334, 0410, 0},
	{0760404, 0310, 0},
};
int udcnt = 4;
struct ud ud[4];
extern struct msportsw udport;
extern struct msportsw bdport;
int nmsport = 2;
struct msportsw *msportsw[] = {
	&udport,	/* 0 */
	&bdport,	/* 1 */
};
struct msaddr taaddr[] = {
	{0, 0, 0},
	{3, 0, 0},
};
int tacnt = 2;
struct tatape tatape[2];
struct buf tabuf[2];
struct ubaddr kmcaddr[] = {
	{0760200, 0600, 0},
};
int kmccnt = 1;
struct kmc kmc[1];
int kdicnt = 1;
struct kdikmc kdikmc[1];
struct kmcdk k[1];
int kbcnt = 96;
struct kb kb[96];
char kbstate[96];
struct ubaddr dkaddr[] = {
	{0767770, 0400, 0},
};
int dkcnt = 256;
struct dk dk[256];
char dkstate[256];
struct ubaddr dzaddr[] = {
	{0760100, 0300, 0},
};
int dzcnt = 8;
struct dz dz[8];
struct ubaddr cureaddr[] = {
	{0776070, 0210, 0},
};
int curecnt = 1;
struct ubaddr rcureaddr[] = {
	{0776070, 00, 0},
};
int rcurecnt = 1;
struct msaddr raaddr[] = {
	{0, 1, 0},
	{0, 1, 1},
	{0, 1, 2},
	{0, 1, 3},
	{1, 0, 0},
	{1, 0, 1},
	{1, 0, 2},
	{1, 0, 3},
};
int racnt = 8;
struct radisk radisk[8];
struct buf rabuf[8];
struct biaddr bnaaddr[] = {
	{4, 0x40, 0x0, 2},
};
int bnacnt = 1;
struct bnactl bna[1];
struct bnabuf bnabuf[1];
int cncnt = 0;
int ipcnt = 32;
struct ipif ipif[32];
struct ipif *ipifsort[32];
int udpcnt = 16;
struct udp udpconn[16];
int tcpcnt = 32;
struct tcpcb tcpcb[32];
int arpcnt = 128;
struct ip_arp ip_arps[128];
int ttycnt = 128;
struct ttyld ttyld[128];
int msgcnt = 300;
struct imesg mesg[300];
int rdkcnt = 0;
int xpcnt = 0;
int dkpcnt = 256;
struct dkp dkp[256];
int cdkpcnt = 0;
int bufldcnt = 4;
struct bufld bufld[4];
int fscnt = 15;
struct mount fsmtab[15];
int rootfstyp = 0;
dev_t rootdev = makedev(7, 64);
dev_t swapdev = makedev(4, 0);
struct swdevt swdevt[] = {
	{makedev(7, 1), 20480L},
	{makedev(7, 9), 20480L},
	{makedev(7, 26), 249848L},
	{makedev(7, 28), 249848L},
};
int nswdevt = 4;
extern int bddump();
int (*dumprout)() = bddump;
int dumpunit = 9219;
long dumplow = 280568;
long dumpsize = 249848;
