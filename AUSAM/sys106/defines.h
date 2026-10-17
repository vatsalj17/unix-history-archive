#ifdef _1170
#define UNIBUS_MAP
#endif _1170
#define _1145
#define FPU
#define DEVSTART
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
#define QMOUNT
#define ACCESS
#ifndef GPROCS
#define GETTAB
#endif GPROCS
#define NEWCOPYSEG
#define CIRCULAR_PIPE
