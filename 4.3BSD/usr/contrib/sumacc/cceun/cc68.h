#ifndef MAXNAMLEN
#define MAXNAMLEN 255
#endif

# define LucasKludge ""
# define BinDirectory "/usr/macintosh/bin/"
/* run-time start-up */
char	*crt0 = "/usr/macintosh/lib/crtsun.b";
char	*maccrt0 = "/usr/macintosh/lib/crt0.b";
char	*profcrt0 = "/usr/macintosh/lib/mcrt0.b";
char	*macroot = "/usr/macintosh/lib/crtmac.b";
char	*teamroot = "/usr/macintosh/lib/teamroot.b";
char	*xteamroot = "/usr/macintosh/xlib/teamroot.b";
char	*sunincludes = "-I/usr/macintosh/include";
char	*sunxincludes = "-I/usr/macintosh/xinclude";
char	*localincludes = "-I/usr/macintosh/include";
char	*dmincludes = "-I/usr/macintosh/include";
char	*asheader = "/usr/macintosh/lib/a68.hdr";	/* assembler header file */
char	*libmac = "/usr/macintosh/lib/libmac.a";
char	*libc68 = "/usr/macintosh/lib/libc68.a";
char	*libv68 = "/usr/macintosh/lib/libv68.a";
char	*libg68 = "/usr/macintosh/lib/libg68.a";
char	*defines = "-DMC68000";	/* tell cpp this is a 68000 */
