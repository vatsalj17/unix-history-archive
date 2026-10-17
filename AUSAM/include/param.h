
#define NBUF 48
#define NPROC 150
#define NINODE 180
#define NFILE 250
#define NMOUNT 8
#define NEXEC 4
#define MAXMEM (64*32)
#define SSIZE 20
#define SINCR 20
#define NOFILE 15
#define TTYHOG 256
#define CMAPSIZ 150
#define SMAPSIZ 300
#define UMAPSIZ 32
#define NCALL 20
#define NTEXT 30
#define NCLIST 300
#define HZ 50
#define NTFREE 2
#define NRAWBUFS 10
#define CMASKSIZE 2
#define SCHMAG 5
#define PRIORATE 5
#define PSWP -100
#define PINOD -90
#define PRIBIO -50
#define PRIREDIRECT -1
#define PDELAY -1
#define PPIPE 1
#define PWAIT 30
#define PPV 30
#define PLOCK 40
#define PSLEP 50
#define PUSER 64
#define NSIG 20
#define SIGHUP 1
#define SIGINT 2
#define SIGQIT 3
#define SIGINS 4
#define SIGTRC 5
#define SIGIOT 6
#define SIGEMT 7
#define SIGFPT 8
#define SIGKIL 9
#define SIGBUS 10
#define SIGSEG 11
#define SIGSYS 12
#define SIGPIPE 13
#define SIGTERMINATE 14
#define SIGTIMEOUT 15
#define SIGCPUTL 16
#define SIGMEMPAR 17
#define USIZE 16
#define NULL 0
#define NODEV (-1)
#define ROOTINO 1
#define DIRSIZ 14
struct
{
char lobyte;
char hibyte;
};
struct
{
int integ;
};
struct
{
char byt[];
};
struct
{
unsigned unsignd;
};
struct
{
int hiint;
int loint;
};
struct
{
int intarray[];
};
struct
{
int *integptr;
};
#define PS 0177776
#define KL 0177560
#define SW 0177570
#define SLR 0177774
#define KA5 0172372
#define ka5 KA5->integ
#define guid(ip) ((ip->i_uidh << 8) | (ip->i_uidl & 0377))
#define MAXUSERS 25
