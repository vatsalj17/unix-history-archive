/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

extern char	*get_end;
extern char	*get_ptr;
extern char	get_buff[];

extern int out_fid;
#include <fio.h>


#define put(c)	if(out_fid!=NO_FID){Fputc(out_fid, (int)(c));}


/*
 *	put a 4 byte integer, native byte order
 */

#define	putnum(j)	if(out_fid!=NO_FID){ long _ = j; \
			    Fwrite(out_fid, (char*)&_, (long)sizeof(_));}

