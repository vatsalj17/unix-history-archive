#define NBUF 20
#define NPROC 50
#define NINODE 80
#define NFILE 80
#define NMOUNT 6
#define NEXEC 3
#define MAXMEM (64*32)
#define SSIZE 15
#define SINCR 16
#define NOFILE 15
#define TTYHOG 256
#define CMAPSIZ 100
#define SMAPSIZ 100
#define NCALL 20
#define NTEXT 40
#define NCLIST 70
#define HZ 50
#ifdef NEW_TIMEOUT
#define NTFREE 2
#endif
#ifdef RAW_BUFFER_POOL
#define NRAWBUFS 6
#endif RAW_BUFFER_POOL
#ifdef AUSAML
#define CMASKSIZE 4
#endif AUSAML
#ifdef MANY_USERS
#define SCHMAG 5
#define PRIORATE 5
#else
#define SCHMAG 10
#define PRIORATE 16
#endif MANY_USERS
#ifdef TTY_SUSER
#define TTY_MAXSPD 11
#endif TTY_SUSER
#define PSWP -100
#define PINOD -90
#define PRIBIO -50
#ifdef DELAY
#ifdef TTY_CONNECT
#define PRIREDIRECT -1
#endif TTY_CONNECT
#define PDELAY -1
#endif
#define PPIPE 1
#ifdef MORE_USER_PRIORITIES
#define PWAIT 30
#ifdef SHARED_DATA
#define PPV 30
#endif
#ifdef LOCKING
#define PLOCK 40
#endif
#define PSLEP 50
#define PUSER 64
#else
#define PWAIT 40
#ifdef SHARED_DATA
#define PPV 40
#endif
#ifdef LOCKING
#define PLOCK 50
#endif
#define PSLEP 90
#define PUSER 100
#endif MORE_USER_PRIORITIES
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
#ifdef TIME_LIMITS
#define SIGTIMEOUT 15
#define SIGCPUTL 16
#endif
#ifdef MEM_PAR_INTR
#define SIGMEMPAR 17
#endif MEM_PAR_INTR
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
#ifdef STACK_LIMIT
#define SLR 0177774
#endif STACK_LIMIT
#ifdef MEM_PAR_INTR & _1140
#define PARMEMCSR 0172100
#endif MEM_PAR_INTR & _1140
#ifdef AUSAM16
#define guid(ip) ((ip->i_uidh << 8) | (ip->i_uidl & 0377))
#endif AUSAM16
#ifdef AUSAML
#define MAXUSERS 20
#endif AUSAML
