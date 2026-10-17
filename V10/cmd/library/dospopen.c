/*	library:dospopen.c	1.2	*/
#include "sccsid.h"
VERSION(@(#)library:dospopen.c	1.2)

/* popen/pclose: simple MS-DOS piping scheme to imitate UNIX pipes */

/* information needed between popen and pclose */
#include <stdio.h>

static char *prgname[32];          /* program name if write pipe */
static int pipetype[32];           /* 1=read 2=write */

/* open a pipe */
FILE *popen(prg,type)
char *prg,*type;
{ FILE *p;
  int ostdout, ostdin;

  switch(*type) {

    /* for write style pipe, pclose handles program execution */
    case 'w' :
      if ((p= fopen("\\pipe.tmp","w")) != NULL) {
        pipetype[fileno(p)]= 1;
        prgname[fileno(p)]= prg;
      }
      return(p);

/* read pipe must create tmp file, set up stdout to point to the temp
 * file, and run the program.  note that if the pipe file cannot be
 * opened, it'll return a condition indicating pipe failure, which is
 * fine.
 */

    case 'r' :
      if ((p= fopen("\\pipe.tmp","w")) != NULL) {
        pipetype[fileno(p)]= 2;
        ostdout= dup(fileno(stdout));   /* we need this later */
        dup2(fileno(stdout),fileno(p)); /* substitute for stdout */
        system(prg);                    /* run the program */
        dup2(fileno(stdout),ostdout);   /* repair stdout */
        fclose(p);                      /* close redirected stdout */
        return(fopen("\\pipe.tmp","r"));     /* return pointer to tmp file */
      }
      return(NULL);                     /* everyone has their problems */

    /* screwy call or unsupported type */
    default :
      printf("popen: unknown pipe style\n");
      exit(1);
  }
}

/* close a pipe */
void pclose(p)
FILE *p;
{ int n;
  int ostdout, ostdin;
  FILE *p2;

  switch(pipetype[fileno(p)]) {

/* close the temp file, open again as read, redirect stdin from that
 * file, run the program, then clean up.
 */

    case 1 :
      n= fileno(p);
      fclose(p);
      if ((p2= fopen("\\pipe.tmp","r")) != NULL) {
        ostdin= dup(fileno(stdin));     /* save stdin for later */
        dup2(fileno(stdin),fileno(p2)); /* redirect to tmp file */
        system(prgname[n]);             /* run the program */
        dup2(fileno(stdin),ostdin);     /* repair stdin */
        fclose(p2);
        unlink("\\pipe.tmp");            /* erase tmp file */
        return;
      }
      printf("pclose: could not reopen temporary file\n");
      exit(1);

    /* close the temp file and remove it */
    case 2 :
      n= fileno(p);        /* get file number for unlink */
      fclose(p);           /* close the file */
      unlink("\\pipe.tmp"); /* erase the file */
      return;

    /* if we're neither read nor write, we have problems */
    default :
      printf("pclose: internal error\n");
      exit(1);
  }
}

