/* dtsock.h		Socket info */

#ifndef makedev
#include <sys/types.h>
#endif makedev

#include <sys/buf.h>
#include <sys/dtdefs.h>
#include <sys/ioctl.h>
#include <sys/dtio.h>

#define	DT_ANYSOCK		-1
#define	DT_ANYSTART		32
#define	DT_ANYEND		63
#define	DT_DEVNAME		"/dev/dt/"

/* Assigned sockets */
#define	DT_MAINTSOCK	1
#define	DT_VMDSOCK		3
#define	DT_RINSDSOCK	4
#define	DT_DROPDSOCK	5
#define	DT_TESTSOCK		10
#define	DT_MAZEWSOCK	21
#define	DT_MAZERSOCK	22

typedef struct {
	struct dt_head	head;
	char	body[DT_BODYSZ];
} dtpacket;
