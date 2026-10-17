/*
 * File: rsys.c
 *  Contents: getstr, host, putstr
 */

#include "../h/rt.h"

/*
 * getstr - read a line into buf from file fd.  At most maxi characters
 *  are read.  getstr returns the length of the line, not counting
 *  the newline.
 */

getstr(buf, maxi, fd)
register char *buf;
int maxi;
FILE *fd;
   {
   register int c, l;

   l = 0;
   while ((c = getc(fd)) != '\n') {
      if (c == EOF)
	 if (l > 0) return l;
	 else return -1;
      *buf++ = c;
      if (++l >= maxi)
         break;
      }
   return l;
   }


#ifdef UtsName
#include <sys/utsname.h>
#endif UtsName

/*
 * iconhost - return some sort of host name into the buffer pointed at
 *  by hostname.  This code accommodates several different host name
 *  fetching schemes.
 */
iconhost(hostname)
char *hostname;
   {
#ifdef WhoHost
   /*
    * The host name is in /usr/include/whoami.h. (V7, 4.[01]bsd)
    */
   whohost(hostname);
#endif WhoHost

#ifdef UtsName
   {
   /*
    * Use the uname system call.  (System III & V)
    */
   struct utsname uts;
   uname(&uts);
   strcpy(hostname,uts.nodename);
   }
#endif UtsName

#ifdef GetHost
   /*
    * Use the gethostname system call.  (4.2bsd)
    */
   gethostname(hostname,MaxCvtLen);
#endif GetHost

#ifdef VMS
   /*
    * VMS has its own special logic
    */
   char *h;
   if (!(h = getenv("ICON$HOST")) && !(h = getenv("SYS$NODE")))
      h = "VAX/VMS";
   strcpy(hostname,h);
#endif VMS

#ifdef HostStr
   /*
    * The string constant HostStr contains the host name.
    */
   strcpy(hostname,HostStr);
#endif HostStr
   }

#ifdef WhoHost
#define HdrFile "/usr/include/whoami.h"
/*
 * whohost - look for a line of the form
 *  #define sysname "name"
 * in HdrFile and return the name.
 */
whohost(hostname)
char *hostname;
   {
   char buf[BUFSIZ];
   FILE *fd;

   fd = fopen(HdrFile, "r");
   if (fd == NULL) {
      sprintf(buf, "Cannot open %s, no value for &host\n", HdrFile);
      syserr(buf);
   }
#ifndef VMS
   setbuf(fd,NULL);
#endif VMS

   for (;;) {   /* each line in the file */
      if (fgets(buf, sizeof buf, fd) == NULL) {
         sprintf(buf, "No #define for sysname in %s, no value for &host\n", HdrFile);
         syserr(buf);
      }
      if (sscanf(buf,"#define sysname \"%[^\"]\"", hostname) == 1) {
         fclose(fd);
         return;
      }
   }
   }
#endif WhoHost


/*
 * Print l characters starting at s on file f.
 */

putstr(f, s, l)
register FILE *f;
register char *s;
register word l;
   {
   while (l--)
      putc(*s++, f);
   }
