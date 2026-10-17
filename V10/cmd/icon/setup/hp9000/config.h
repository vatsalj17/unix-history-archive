/*
 * Icon configuration. This file is produced when the Icon system is
 * built by icon-setup.
 */

/*
 * The following #defines may occur:
 *
 *	DEBUG		debugging code
 *	MEMMON		code for monitoring memory management (for AED display)
 *	SYSINFO		code for collecting system information via the sysinfo()
 *			   function (only complete for the VAX)
 *	VMS		code specific to the VAX-11 under VMS
 */

#define Ident(x) x
#define Cat(x,y) Ident(x)y
#define VERSION		"Icon Version 6.0. January 20, 1986"
#define OPTIONS "-interpex -host gethost -vfork"
#define GetHost 1
#define IntBin "/users/whm/v6/bin"
#define Iconx "/users/whm/v6/bin/iconx"
#define IconxHdr "/users/whm/v6/bin/iconx.hdr"
#define Fork fork
#define HZ 60
#define VAX 1
#define index strchr
#define rindex strrchr
#define IntSize	32
#define LargeMem	 1
#define MaxHdr  2048
