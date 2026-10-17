/*
 *	SEEK
 *
 *	Seek to position in stream
 *
 *	Stephen B. Wampler
 *
 *	Last modified 5/2/86 by Ralph E. Griswold
 *
*/

#include "../h/rt.h"

/*
 * seek(file,offset,start) - seek to offset byte from start in file.
 */

FncDcl(seek,3)
   {
   long l1, l2;
   int status;
   FILE *fd;
   long ftell();

   if (Arg1.dword != D_File)
      runerr(106);

   defint(&Arg2, &l1, 0);
   defshort(&Arg3, 0);

   fd = BlkLoc(Arg1)->file.fd;

   if ((BlkLoc(Arg1)->file.status == 0) ||
       (fseek(fd, l1, Arg3.vword.integr) == -1))
      Fail;
   Mkint(ftell(fd), &Arg0);
   Return;
   }
