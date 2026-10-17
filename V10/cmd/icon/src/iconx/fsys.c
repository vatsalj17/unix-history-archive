/*
 * File: fsys.c
 *  Contents: close, exit, open, read, reads, stop, system, write, writes
 */

#include "../h/rt.h"

/*
 * close(f) - close file f.
 */

FncDcl(close,1)
   {
   register int i;

   /*
    * f must be a file.
    */
   if (Arg1.dword != D_File)
      runerr(105, &Arg1);

   /*
    * If f has a buffer associated with it, "free" it.
    */
   for (i = 0; i < numbufs; i++) {
      if (bufused[i] == BlkLoc(Arg1)->file.fd) {
	 bufused[i] = NULL;
	 break;
	 }
      }

   /*
    * Close f, using fclose or pclose as appropriate.
    */
#ifndef MSDOS
   if (BlkLoc(Arg1)->file.status & Fs_Pipe)
      pclose(BlkLoc(Arg1)->file.fd);
   else
#endif MSDOS
      fclose(BlkLoc(Arg1)->file.fd);
   BlkLoc(Arg1)->file.status = 0;

   /*
    * Return the closed file.
    */
   Arg0 = Arg1;
   Return;
   }


/*
 * exit(status) - exit process with specified status, defaults to 0.
 */

FncDcl(exit,1)
   {
   defshort(&Arg1, NormalExit);
   c_exit((int)IntVal(Arg1));
   }


/*
 * open(s1,s2) - open file s1 with specification s2.
 */
FncDcl(open,2)
   {
   register word slen;
   register int i;
   register char *s;
   int status;
#ifndef MSDOS
   char sbuf1[MaxCvtLen], sbuf2[MaxCvtLen], mode[3];
#else MSDOS
   char sbuf1[MaxCvtLen], sbuf2[MaxCvtLen], mode[4];
   char untranslated;
#endif MSDOS
   FILE *f;
   extern struct b_file *alcfile();
   extern char *alcstr();
#ifndef MSDOS
   extern FILE *fopen(), *popen();
#else MSDOS
   extern FILE *fopen();
#endif MSDOS
   /*
    * s1 must be a string and a C string copy of it is also needed.
    *  Make it a string if it isn't one; make a C string if s1 is
    *  a string.
    */
   switch (cvstr(&Arg1, sbuf1)) {

      case Cvt:
	 strreq(StrLen(Arg1));
	 StrLoc(Arg1) = alcstr(StrLoc(Arg1), StrLen(Arg1));
	 break;

      case NoCvt:
	 qtos(&Arg1, sbuf1);
	 break;

      default:
	 runerr(103, &Arg1);
      }
   /*
    * s2 defaults to "r".
    */
   defstr(&Arg2, sbuf2, &letr);

   blkreq((word)sizeof(struct b_file));
   status = 0;
#ifdef MSDOS
   untranslated = 0;
#endif MSDOS
   /*
    * Scan s2, setting appropriate bits in status.  Produce a runerr
    *  if an unknown character is encountered.
    */
   s = StrLoc(Arg2);
   slen = StrLen(Arg2);
   for (i = 0; i < slen; i++) {
      switch (*s++) {
	 case 'a': case 'A':
	    status |= Fs_Write|Fs_Append;
	    continue;
	 case 'b': case 'B':
	    status |= Fs_Read|Fs_Write;
	    continue;
	 case 'c': case 'C':
	    status |= Fs_Create|Fs_Write;
	    continue;
#ifndef MSDOS
	 case 'p': case 'P':
	    status |= Fs_Pipe;
	    continue;
#else MSDOS
	 case 't': case 'T':   /* Add a translated (default) */
	    untranslated = 0;
	    continue;
	 case 'u': case 'U':   /* Add a translated (default) */
	    untranslated = 1;
	    continue;
#endif MSDOS
	 case 'r': case 'R':
	    status |= Fs_Read;
	    continue;
	 case 'w': case 'W':
	    status |= Fs_Write;
	    continue;
	 default:
	    runerr(209, &Arg2);
	 }
      }

   /*
    * Construct a mode field for fopen/popen.
    */
   mode[0] = '\0';
   mode[1] = '\0';
   mode[2] = '\0';
#ifdef MSDOS
   mode[3] = '\0';
#endif MSDOS
   if ((status & (Fs_Read|Fs_Write)) == 0)   /* default: read only */
      status |= Fs_Read;
   if (status & Fs_Create)
      mode[0] = 'w';
   else if (status & Fs_Append)
      mode[0] = 'a';
   else if (status & Fs_Read)
      mode[0] = 'r';
   else
      mode[0] = 'w';
#ifndef MSDOS
   if ((status & (Fs_Read|Fs_Write)) == (Fs_Read|Fs_Write))
      mode[1] = '+';
#else MSDOS
   if ((status & (Fs_Read|Fs_Write)) == (Fs_Read|Fs_Write)) {
      mode[1] = '+';
      mode[2] = untranslated ? 'b' : 't';
   }
   else mode[1] = untranslated ? 'b' : 't';
#endif MSDOS

   /*
    * Open the file with fopen or popen.
    */
#ifndef MSDOS
   if (status & Fs_Pipe) {
      if (status != (Fs_Read|Fs_Pipe) && status != (Fs_Write|Fs_Pipe))
	 runerr(209, &Arg2);
      f = popen(sbuf1, mode);
      }
   else
#endif MSDOS
      f = fopen(sbuf1, mode);
   /*
    * Fail if the file can't be opened.
    */
   if (f == NULL)
      Fail;
   /*
    * If the file isn't a terminal and a buffer is available, assign
    *  it to the file.
    */
#ifndef VMS
   if (!isatty(fileno(f))) {
      for (i = 0; i < numbufs; i++)
	 if (bufused[i] == NULL)
	    break;
      if (i < numbufs) {	      /* Use buffer if any free. */
	 setbuf(f, bufs[i]);
	 bufused[i] = f;
	 }
      else
	 setbuf(f, NULL);
      }
   else
      setbuf(f, NULL);
#endif VMS
   /*
    * Return the resulting file value.
    */
   Arg0.dword = D_File;
   BlkLoc(Arg0) = (union block *) alcfile(f, status, &Arg1);
   Return;
   }


/*
 * read(f) - read line on file f.
 */
FncDcl(read,1)
   {
   register word slen;
   int status;
   static char sbuf[MaxReadStr];
   FILE *f;
   extern char *alcstr();

   /*
    * Default f to &input.
    */
   deffile(&Arg1, &input);
   /*
    * Get a pointer to the file and be sure that it's open for reading.
    */
   f = BlkLoc(Arg1)->file.fd;
   status = BlkLoc(Arg1)->file.status;
   if ((status & Fs_Read) == 0)
      runerr(212, &Arg1);

   /*
    * Use getstr to read a line from the file, failing if getstr
    *  encounters end of file.
    */
   if ((slen = getstr(sbuf,MaxReadStr,f)) < 0)
      Fail;
   /*
    * Allocate the string read and make Arg0 a descriptor for it.
    */
   strreq(slen);
   StrLen(Arg0) = slen;
   StrLoc(Arg0) = alcstr(sbuf,slen);
   Return;
   }


/*
 * reads(f,i) - read i characters on file f.
 */
FncDcl(reads,2)
   {
   register int cnt;
   int status;
   FILE *f;

   /*
    * f defaults to &input and i defaults to 1 (character).
    */
   deffile(&Arg1, &input);
   defshort(&Arg2, 1);

   /*
    * Get a pointer to the file and be sure that it's open for reading.
    */
   f = BlkLoc(Arg1)->file.fd;
   status = BlkLoc(Arg1)->file.status;
   if ((status & Fs_Read) == 0)
      runerr(212, &Arg1);

   /*
    * Be sure that a positive number of bytes is to be read.
    */
   if ((cnt = IntVal(Arg2)) <= 0)
      runerr(205, &Arg2);

   /*
    * Ensure that enough space for the string exists and read it directly
    *  into the string space.  (By reading directly into the string space,
    *  no arbitrary restrictions are placed on the size of the string that
    *  can be read.)  Make Arg0 a descriptor for the string and return it.
    */
   strreq((word)cnt);
   if (strfree + cnt > strend)
      syserr("string allocation botch");
   StrLoc(Arg0) = strfree;
   if ((cnt = fread(StrLoc(Arg0), sizeof(char), cnt, f)) <= 0)
      Fail;
   StrLen(Arg0) = cnt;
   strfree += cnt;
   Return;
   }


/*
 * stop(a,b,...) - write arguments (starting on error output) and stop.
 */

FncDclV(stop)
    {
   register word n;
   char sbuf[MaxCvtLen];
   struct descrip arg;
   FILE *f;

   f = stderr;
   /*
    *	Loop through arguments.
    */

   for (n = 1; n <= nargs; n++) {
      arg = Arg(n);
      if (arg.dword == D_File) {
	 if (n > 1)
	    putc('\n', f);
	 if ((BlkLoc(arg)->file.status & Fs_Write) == 0)
	    runerr(213, &arg);
	 f = BlkLoc(arg)->file.fd;
	 }
      else {
	 if (n == 1 && (k_output.status & Fs_Write) == 0)
	    runerr(213, NULL);
	 if (ChkNull(arg))
	    arg = emptystr;
	 if (cvstr(&arg, sbuf) == NULL)
	    runerr(109, &arg);
	 putstr(f, StrLoc(arg), StrLen(arg));
	 }
      }

   putc('\n', f);
   c_exit(ErrorExit);
   }


/*
 * system(s) - execute string s as a system command.
 */

FncDcl(system,1)
   {
   char sbuf1[MaxCvtLen], sbuf2[MaxCvtLen];
#ifdef VMS
    struct { long size; char *ptr; } descr;
    int status;
#endif VMS

   /*
    * s must be string and smaller than MaxCvtLen characters long.
    */
   if (cvstr(&Arg1, sbuf1) == NULL)
      runerr(103, &Arg1);
   if (StrLen(Arg1) >= MaxCvtLen)
      runerr(210, &Arg1);
   qtos(&Arg1, sbuf2);

   /*
    * Pass the C string made by qtos to the UNIX system() function and
    *  return the exit code of the command as the result of system().
    */
#ifndef VMS
   Mkint((long)((system(sbuf2) >> 8) & 0377), &Arg0);
   Return;
#else
    descr.ptr = sbuf2; descr.size = StrLen(Arg1); status = 0;
    status = lib$spawn(&descr,0,0,0,0,0, &status,0,0,0);
    Mkint((long)status|ErrorExit, &Arg0);
    Return;
#endif VMS
   }


/*
 * write(a,b,...) - write arguments.
 */
FncDclV(write)
   {
   register word n;
   char sbuf[MaxCvtLen];
   struct descrip arg;
   FILE *f;
   extern char *alcstr();

   f = stdout;
   arg = emptystr;

   /*
    * Loop through the arguments.
    */
   for (n = 1; n <= nargs; n++) {
      arg = Arg(n);
      if (arg.dword == D_File)	{/* Current argument is a file */
	 /*
	  * If this isn't the first argument, output a newline to the current
	  *  file and flush it.
	  */
	 if (n > 1) {
	    putc('\n', f);
	    fflush(f);
	    }
	 /*
	  * Switch the current file to the file named by the current argument
	  *  providing it is a file.  arg is made to be a empty string to
	  *  avoid a special case.
	  */
	 if ((BlkLoc(arg)->file.status & Fs_Write) == 0)
	    runerr(213, &arg);
	 f = BlkLoc(arg)->file.fd;
	 arg = emptystr;
	 }
      else {	/* Current argument is a string */
	 /*
	  * On first argument, check to be sure that &output is open
	  *  for output.
	  */
	 if (n == 1 && (k_output.status & Fs_Write) == 0)
	    runerr(213, NULL);
	 /*
	  * Convert the argument to a string, defaulting to a empty string.
	  */
	 if (ChkNull(arg))
	    arg = emptystr;
	 if (cvstr(&arg, sbuf) == NULL)
	    runerr(109, &arg);
	 /*
	  * Output the string.
	  */
	 putstr(f, StrLoc(arg), StrLen(arg));
	 }
      }
   /*
    * Append a newline to the file and flush it.
    */
   putc('\n', f);
   fflush(f);
   /*
    * If the beginning of the last string output lies in sbuf,
    *  allocate it as a real string.  Note that some of the string
    *  conversions don't always leave the converted string at the
    *  start of the conversion buffer, hence the range check.
    */
   if (StrLoc(arg) >= sbuf && StrLoc(arg) < sbuf + MaxCvtLen) {
      strreq(StrLen(arg));
      StrLoc(arg) = alcstr(StrLoc(arg), StrLen(arg));
      }
   /*
    * Return the string corresponding to the last argument.
    */
   Arg(0) = arg;
   Return;
   }


/*
 * writes(a,b,...) - write arguments without newline terminator.
 */

FncDclV(writes)
   {
   register word n;
   char sbuf[MaxCvtLen];
   struct descrip arg;
   FILE *f;
   extern char *alcstr();

   f = stdout;
   arg = emptystr;

   /*
    * Loop through the arguments.
    */
   for (n = 1; n <= nargs; n++) {
      arg = Arg(n);
      if (arg.dword == D_File)	{/* Current argument is a file */
	 /*
	  * Switch the current file to the file named by the current argument
	  *  providing it is a file.  arg is made to be a empty string to
	  *  avoid a special case.
	  */
	 if ((BlkLoc(arg)->file.status & Fs_Write) == 0)
	    runerr(213, &arg);
	 f = BlkLoc(arg)->file.fd;
	 arg = emptystr;
	 }
      else {	/* Current argument is a string */
	 /*
	  * On first argument, check to be sure that &output is open
	  *  for output.
	  */
	 if (n == 1 && (k_output.status & Fs_Write) == 0)
	    runerr(213, NULL);
	 /*
	  * Convert the argument to a string, defaulting to a empty string.
	  */
	 if (ChkNull(arg))
	    arg = emptystr;
	 if (cvstr(&arg, sbuf) == NULL)
	    runerr(109, &arg);
	 /*
	  * Output the string and flush the file.
	  */
	 putstr(f, StrLoc(arg), StrLen(arg));
	 fflush(f);
	 }
      }
   /*
    * If the beginning of the last string output lies in sbuf,
    *  allocate it as a real string.  Note that some of the string
    *  conversions don't always leave the converted string at the
    *  start of the conversion buffer, hence the range check.
    */
   if (StrLoc(arg) >= sbuf && StrLoc(arg) < sbuf + MaxCvtLen) {
      strreq(StrLen(arg));
      StrLoc(arg) = alcstr(StrLoc(arg), StrLen(arg));
      }
   /*
    * Return the string corresponding to the last argument.
    */
   Arg(0) = arg;
   Return;
   }
