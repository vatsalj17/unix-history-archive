#include "../h/config.h"
#include "../h/cpuconf.h"
#include "../h/header.h"
#ifndef NoHeader
#include <sys/types.h>
#include <sys/stat.h>
#include "../h/paths.h"
#ifndef Iconx
#define Iconx IconxPath
#endif Iconx
#define MaxArgs 500
char *pptr;
main(argc,argv)
int argc;
char **argv;
   {

   char *path, *getenv();
   char file[256];
   char *name;
   char *argvx[MaxArgs];
   
   name = argv[0];
   do 
      argvx[argc+1] = argv[argc];
   while (argc--);
   argv = argvx;
   /*
    * If the name contains any slashes we skip the path search and
    *  just try to run the file.
    */
   if (index(name,'/'))
      doiconx(argv,name);
   
   pptr = path = getenv("PATH");
   if (index(path,'.') == 0) {
      if (canrun(name))
         doiconx(argv,name);
      }
   while (trypath(name,file)) {
      if (canrun(file))
         doiconx(argv,file);
      }
   /*
    * If we can't find it, we assume that it must exist somewhere
    *  and infer that it's in the current directory.
    */
   if (canrun(name))
      doiconx(argv,name);
   exit(100);
   }
canrun(file)
char *file;
   {
   struct stat statb;
   if (access(file,5) == 0) {
      stat(file,&statb);
      if (statb.st_mode & S_IFREG)
         return 1;
      }
   return 0;
   }
doiconx(av,file)
char **av; char *file;
   {
   av[0] = "-iconx";
   av[1] = file;
   execv(Iconx,av);
   exit(200);
   }
trypath(name,file)
char *name, *file;
   {
   char c;
   
   while (*pptr == ':')
      pptr++;
   if (!*pptr)
      return 0;
   do {
      c = (*file++ = *pptr++);
      } while (c != ':' && c);
   pptr--;
   file--;
   
   *file++ = '/';
   while (*file++ = *name++);
   *file = 0;
   return 1;
   }
exit(c)
   {
   _exit(c);
   }
#else NoHeader
main()
   {
   }
#endif NoHeader
