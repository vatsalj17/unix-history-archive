/*
 * Routines for processing .u2 files.
 */

#include "ilink.h"
#include "opcode.h"
#include "../h/version.h"

int trace = 0;			/* initial setting of &trace */
int nrecords = 0;		/* number of records in program */

/*
 * globals reads the global information from infile (.u2) and merges it with
 *  the global table and record table.
 */
globals(i)
int i;
   {
   register char *id;
   register int n, op;
   int k;
   int implicit;
   char *name;
   struct gentry *gp, *glocate();
   extern char *getid(), *getstr();
   extern struct gentry *putglob();
   if (getop(&name) != Op_Version) {
      fprintf(stderr,"ucode file has no version identification\n");
      exit(ErrorExit);
      }
   id = getid();		/* get version number of ucode */
   newline();
   if (strcmp(id,UVersion)) {
      fprintf(stderr,"ucode file version mismatch\n");
      fprintf(stderr,"\tucode version: %s\n",id);
      fprintf(stderr,"\texpected version: %s\n",UVersion);
      exit(ErrorExit);
      }
   while ((op = getop(&name)) != EOF) {
      switch (op) {
         case Op_Record:	/* a record declaration */
            id = getid();	/* record name */
            n = getdec();	/* number of fields */
            newline();
            gp = glocate(id);
            /*
             * It's ok if the name isn't already in use or if the
             *  name is just used in a "global" declaration.  Otherwise,
             *  it is an inconsistent redeclaration.
             */
            if (gp == NULL || (gp->g_flag & ~F_Global) == 0) {
               putglob(id, F_Record, n, ++nrecords);
               while (n--) {	/* loop reading field numbers and names */
                  k = getdec();
                  putfield(getid(), nrecords, k);
                  newline();
                  }
               }
            else {
               err(id, "inconsistent redeclaration", 0);
               while (n--)
                  newline();
               }
            break;

         case Op_Impl:		/* undeclared identifiers should be noted */
            if (getop(&name) == Op_Local)
               implicit = 0;
            else
               implicit = F_ImpError;
            break;

         case Op_Trace:		/* turn on tracing */
            trace = -1;
            break;

         case Op_Global:	/* global variable declarations */
            n = getdec();	/* number of global declarations */
            newline();
            while (n--) {	/* process each declaration */
               getdec();	/* throw away sequence number */
               k = getoct();	/* get flags */
               if (k & (F_Proc & ~F_Global))
                  k |= implicit;
               id = getid();	/* get variable name */
               gp = glocate(id);
               /*
                * Check for conflicting declarations and install the
                *  variable.
                */
               if (gp != NULL &&
                   (k & (F_Proc & ~F_Global)) && gp->g_flag != F_Global)
                  err(id, "inconsistent redeclaration", 0);
               else if (gp == NULL || (k & (F_Proc & ~F_Global)))
                  putglob(id, k, getdec(), 0);
               newline();
               }
            break;

         case Op_Link:		/* link the named file */
            name = getstr();	/* get the name and */
            addlfile(name);	/*  put it on the list of files to link */
            newline();
            break;

         default:
            fprintf(stderr, "%s: ill-formed global file %s\n", pname, inname);
            exit(ErrorExit);
         }
      }
   }
