#include "hd.h"
#include "command.h"

#define	MAXLINE	16

/* Print and modify command tables interactively */

options (parm) char **parm; {
	
	int line = 0;		/* Current line number */
	register struct cmdstruct *cmdp;
	register struct classstruct *classp;
	register char **argv;
	struct parmstruct *parmp;

	/* Print command tab in .vshrc format */
	/* If parm present, dump to file and quit */
	erase ();
	if (*parm) {
		close (1);  creat ("vsh.out", 0644);  line= -9999;
	}
	for (;;) {
		for (cmdp = cmdtab; cmdp->cmd_char >= 0; cmdp++) {
			if (cmdp->cmd_proc) {
				for (classp = classtab;
					*classp->cl_name &&
					classp->cl_proc !=
					cmdp->cmd_proc;
					classp++);

				printf ("%c\t%s", cmdp->cmd_char, 
					classp->cl_name);

				for (argv = cmdp->cmd_argv; *argv;)
					printf ("  %s", *argv++);
				
				if (optline (&line) == FAILURE)
					return REPLOT;
			}
		}
		for (parmp = parmtab; parmp->p_name; parmp++) {
			printf ("%s\t%s", parmp->p_name, parmp->p_val);
			if (optline (&line) == FAILURE) return REPLOT;
		}

		if (*parm) leave ();
		if (line != 0 && optcmd () == FAILURE) break;
		erase ();  line = 0;
	}
	return REPLOT;
}
/* Processing for end of each line includes:
	1.  Print the newline.
	2.  If the end of the page, prompt for a command.
*/
optline (line) int *line; {
	int ret;	/* return from optcmd */

	putch (LF);
	if (++*line < MAXLINE) return SUCCESS;

	ret = optcmd (); erase ();  *line = 0;
	return ret;
}

optcmd () {
	char cbuf [STRMAX],	/* Buffer for input parm */
	     *argv [ARGVMAX];	/* Pointers to input parm */
	int argc;		/* Number of parm */
	int ret;		/* Return from readarg */

	int line = 0;		/* Current line num of rcstream */

	printf ("\nType in a new parameter, or\n\
Press ^D to leave.  Press -Return- to display more parameters.\n");
	for (;;) {
		tty_push (COOKEDMODE);
		ret = readarg (stdin, &line, &argc, argv, cbuf);
		tty_pop ();
		if (argc == 0) break;
		if (compe ("quit", argv [0])) {
			ret = FAILURE;  break;
		}
		if (ret != FAILURE) cmdldarg (line, argc, argv);
	}
	return ret;
}
