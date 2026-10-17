
/*  Parameters for newpgml() fork/exec subroutine                         */

/* parameter for fdarray */
#define CLOSEFD -1                /* if equals fdarray[i], close (i);     */


/* parameter for proctyp */
#define PUREXEC 0		  /* simply exec over current process     */
#define FRKEXEC 1		  /* run it in lower process              */
#define SPNEXEC 2		  /* separate new process from old        */

/* parameter for pgmflags */
#define FRKWAIT 1		  /* wait for FRKEXEC to complete         */
#define FRKPIGO 2		  /* Parent's signals off during FRKWAIT  */
#define FRKCIGO 4		  /* Childs signals off before execl      */
#define FRKERRR 8                 /* Give error return, if child exec     */
				  /*  fails; else exit()                  */
#define NUMTRY  30

