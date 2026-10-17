#include <stdio.h>
#include "../h/config.h"
#include "../h/paths.h"
#define MaxArgs 30
#define PathSize 100	/* maximum length of a fully qualified file name */
#ifndef Itran
#define Itran TranPath
#endif
#ifndef Ilink
#define Ilink LinkPath
#endif
#ifndef Iconx
#define Iconx IconxPath
#endif Iconx


char **rfiles;


main(argc,argv,envp)
int argc; char **argv, **envp;
   {
   char **tfiles;
   char **lfiles;
   char **execlist;
   char *tflags[MaxArgs];
   char *lflags[MaxArgs];
   char **xargs;
   int ntf, nlf, nrf, ntflags, nlflags, cflag, quiet;
   char **arg;
   char *base, *getbase();
   char *u1, *u2, *xfile;
   char *rindex(), *mkname();
   char cmd[PathSize];
   
   ntf = nlf = nrf = ntflags = nlflags = cflag = quiet = 0;
   xargs = NULL;
   rfiles = (char **)calloc(2*(argc+10), sizeof(char **));
   tfiles = (char **)calloc(argc+10, sizeof(char **));
   lfiles = (char **)calloc(argc+10, sizeof(char **));
   execlist = (char **)calloc(2*(argc+10), sizeof(char **));

   tflags[ntflags++] = "itran";
   lflags[nlflags++] = "ilink";
   lflags[nlflags++] = "-i";
   lflags[nlflags++] = Iconx;
   xfile = "";
   
   for (arg = &argv[1]; arg <= &argv[argc-1]; arg++) {
      if ((*arg)[0] == '-') switch ((*arg)[1]) {
         case '\0': /* "-" */
            tfiles[ntf++] = *arg;
            lfiles[nlf++] = rfiles[nrf++]
              = "stdin.u1";
            rfiles[nrf++] = "stdin.u2";
            break;
         case 's':
            tflags[ntflags++] = "-s";
            quiet++;
            break;
         case 'o':
            lfiles[nlf++] = "-o";
            xfile = lfiles[nlf++] = *++arg;
            break;
         case 'x':
            xargs = arg++;
            goto argsdone;
         case 'c':
            cflag++;
            break;
         default:
            lflags[nlflags++] = tflags[ntflags++] = *arg;
            break;
            }
      else if (suffix(*arg,".icn")) {
         tfiles[ntf++] = *arg;
         base = getbase(*arg,".icn");
         u1 = mkname(base,".u1");
         u2 = mkname(base,".u2");
         lfiles[nlf++] = rfiles[nrf++] = u1;
         rfiles[nrf++] = u2;
         }
      else if (suffix(*arg,".u1")) {
         lfiles[nlf++] = *arg;
         }
      else {
         fprintf(stderr,"icont: bad argument '%s'\n",*arg);
         exit(ErrorExit);
         }
      }
argsdone:
   if (nlf == 0)
      usage("icont");
   if (!xfile[0])
      xfile = getbase(lfiles[0],".u1");
   
   if (ntf != 0) {
      tflags[ntflags] = NULL;
      tfiles[ntf] = NULL;
      lcat(execlist,tflags,tfiles);
      runit(Itran,execlist,envp);
      }
   if (cflag) {
      exit(NormalExit);
      }
   if (!quiet)
      fprintf(stderr,"Linking:\n");
   execlist[0] = 0;
   lflags[nlflags] = NULL;
   lfiles[nlf] = NULL;
   lcat(execlist,lflags,lfiles);
   runit(Ilink,execlist,envp);
   rmfiles();
   chmod(xfile,0755);
   if (xargs) {
      if (!quiet)
         fprintf(stderr,"Executing:\n");
      xargs[0] = xfile;
      execlist[0] = "iconx";
      execlist[1] = 0;
      lcat(execlist,xargs,0);
      execv(Iconx,execlist);
      }
   exit(NormalExit);
   }


runit(c,a,e)
char *c; char **a, **e;
   {
   int rc;
   if ((rc = docmd(c,a,e)) != 0) {
      rmfiles();
      exit(ErrorExit);
      }
   }

rmfiles()
   {
   char **p;
   for (p = rfiles;  *p;  p++)
#ifndef VMS
      unlink(*p);
#else
      delete(*p);
#endif
   }


suffix(name,suf)
char *name,*suf;
   {
   char *rindex(), *x;

   if (x = rindex(name,'.'))
      return !strcmp(suf,x);
   else
      return 0;
   }


char *
mkname(name,suf)
char *name,*suf;
   {
   char *p, *malloc();
   
   p = malloc(PathSize);
   strcpy(p,name);
   strcat(p,suf);
   return p;
   }


char *
getbase(name,suf)
char *name,*suf;
   {
   char *f,*e, *rindex(), *p, *malloc();
   
   if (f = rindex(name,'/'))
      f++;
#ifdef VMS
   else if ((f = rindex(name,']')) || (f = rindex(name,':')))
      f++;
#endif VMS
   else
      f = name;
   e = rindex(f,'.');
   p = malloc(PathSize);
   strncpy(p,f,e-f);
   return p;
   }


lcat(c,a,b)
int c[],a[],b[];
   {
   int cp,p;
   
   cp = p = 0;
   while (c[cp])
      cp++;
   while (c[cp] = a[p++])
      cp++;
   p = 0;
   if (b)
      while (c[cp++] = b[p++]);
   }


usage(p)
char *p;
   {
   fprintf(stderr,"usage: %s [-c] [-m] [-t] [-u] file ... [-x args]\n",p);
   exit(ErrorExit);
   }


docmd(cmd,argv,envp)
char *cmd, **argv, **envp;
   {
   int rc, stat;
   rc = Fork();
   if (rc == -1) {
      fprintf(stderr,"No more processes\n");
      return 255;
      }
   if (rc == 0) {
      execve(cmd,argv,envp);
      fprintf(stderr,"exec failed on %s\n",cmd);
      _exit(ErrorExit);
      }
   while (rc != wait(&stat));
#ifndef VMS
   return ((stat>>8) & 0xff);
#else VMS
   return (!(stat&1));
#endif VMS
   }


plist(title,list)
char *title, **list;
   {
   char **p;
   printf("\n%s\n",title);
   for (p = list; *p; p++)
      printf("'%s'\n",*p);
   }
