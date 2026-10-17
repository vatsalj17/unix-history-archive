/* We assume that all BSD machines param.h includes types.h  */
#include <sys/param.h>
#ifndef BSD4_2
#define DIR FILE
#define opendir(x) fopen((x), "r")
#define closedir(x) fclose((x))
#include <sys/types.h>
#endif BSD4_2
/* #include <sys/dir.h> */
#include <ndir.h>
#include <sys/stat.h>

#define MAXBASENAME 14 /* should be DIRSIZ but 4.2bsd prohibits that */
#define MAXFULLNAME BUFSIZ
#define MAXNAMESIZE	64	/* /usr/spool/uucp/<14 chars>/<14 chars>+slop */
#define	TRUE	1
#define	FALSE	0

extern struct stat __s_;
#define READANY(f)	((stat((f),&__s_)==0) && ((__s_.st_mode&(0004))!=0) )
#define READSOME(f)	((stat((f),&__s_)==0) && ((__s_.st_mode&(0444))!=0) )

#define WRITEANY(f)	((stat((f),&__s_)==0) && ((__s_.st_mode&(0002))!=0) )
#define DIRECTORY(f)	((stat((f),&__s_)==0) && ((__s_.st_mode&(S_IFMT))==S_IFDIR) )
#define NOTEMPTY(f)	((stat((f),&__s_)==0) && (__s_.st_size!=0) )
