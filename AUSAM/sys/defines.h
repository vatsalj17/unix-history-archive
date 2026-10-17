
#define _1170
#define FPU
#define RHSTART
#define SHARED_DATA
#define ZOMBIE
#define INIT_FIX
#define TIME_LIMITS
#define GPROCS
#ifndef ZOMBIE
#define BETTER_EXIT
#endif ZOMBIE
#define BETTER_TIME
#define EP_ADDRESS
#ifdef _1140
#define BIG_UNIX
#endif
#define VICAR
#define NEW_TIMEOUT
#define CBLOCK_16
#define TTY_TRUE_RAW
#define TTY_HISPEED
#define STACK_LIMIT
#define SMDATE
#define NEW_SLEEP
#ifdef BIG_UNIX
#define ONCE
#endif
#ifdef _1170
#define CRASH_TRACE
#endif
#define RAW_BUFFER_POOL
#define SLOSHED
#define LOWER_TEXT_SWAPS
#define BUFFER_AGING
#define LARGE_FILE_REFERENCES
#define ERROR_LOG
#define NICE_PUTCHAR
#define COOL_NO_SPACE
#define UPRINTS
#define DELAY
#define MAX_PROC
#ifdef MAX_PROC
#endif
#define LRU_INODE
#define MEM_PAR_INTR
#define AUSAM16
#define AUSAML
#define LOCKING
#ifndef AUSAM16 | LOCKING
#define GROUP_ACCESS
#endif
#define MANY_USERS
#define MORE_USER_PRIORITIES
#define BETTER_PANIC
#define TTY_CONNECT
#define TTY_SPECIAL_POWERS
#define SYS_TIME
#define QMOUNT
#define MAPPED_BUFFERS
#ifndef ZOMBIE
#define ZOMBIE
#endif ZOMBIE
#ifndef QMOUNT
#define QMOUNT
#endif QMOUNT
#define MALLOC_UMAP
#define ACCESS
#define SWAP_CHECK
#ifndef GPROCS
#define GETTAB
#endif GPROCS
#define PROCESS_QUEUES
#define NEWCOPYSEG
#define CIRCULAR_PIPE
