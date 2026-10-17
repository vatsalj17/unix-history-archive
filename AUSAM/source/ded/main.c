#include "ded.h"
#include "char.h"
#include "file.h"

extern putout();        /* see savefile call at end of main */

/* line at top of screen, line at end of file */
int topl, maxl;

/* arrays to hold filename information, declared here so
 * that other routines (quit, savefile etc.) can get at information
 */
char filename[ENOUGH];
char tempname[ENOUGH];
char pathname[ENOUGH];
char tailname[ENOUGH];
char pidname[ENOUGH];
char dlogname[ENOUGH];

/* array of strings which disable automatic 'text input' mode */
char *notxtails[]
 { ".c", ".m", ".m11", ".s", ".p", ".out", 0 };

/* array of strings which indicate automatic 'tab output' mode */
char *tbtails[]
 { ".c", ".m", ".m11", ".s", ".p", 0 };

/* variable to stop the bell ringing (option requested by loonies) */
int ringing true;

/* variable to record when modifier option has been used */
int modopt false;

/****************************************************************
 * Start up editor. Parameters are -                            *
 *                                                              *
 * a file name                                                  *
 *                                                              *
 * -b : set 'text input' mode, in which a newline is forced     *
 *      after the right margin                                  *
 *                                                              *
 * -d:                                                          *
 * -e:  start reading from dlogging file                        *
 *                                                              *
 * -m<character>:                                               *
 *      set c_MODIFY to <character>                             *
 *                                                              *
 * -o<letter>:                                                  *
 *      select debug option <letter>                            *
 *                                                              *
 * -s: don't ring the bell (option for paranoiacs)              *
 *                                                              *
 * -t : set 'tab output' mode, in which leading spaces are      *
 *      converted to tabs                                       *
 *                                                              *
 * -u : disable 'text input' mode - i.e. don't give automatic   *
 *      line breaks                                             *
 *                                                              *
 ****************************************************************/

main(argc,argv)
char **argv;
 { int i, j;
    int argp, *p;
    char lbuf[ENOUGH];
    struct inode statbuf;
    register char *s;
    char *s1, *s2;
    register char c;
    int txopt, tbopt;
    int dlogging;

    argp=1;
    filename[0]=0;
    txopt = tbopt = modopt = dlogging = false;

    while (argp<argc)
     { if (*argv[argp]=='-')
	  switch(argv[argp][1])
	   { case 'o':
		if (set_dbug(argv[argp][2]))
		  break;
		else
		  quit( (argv[argp][2]==0 ? "no debug option follows -o" :
					     "invalid debug option %c"),
						argv[argp][2]);

	      case 'b':
		txopt = txin = true;
		break;

	      case 'd':
	      case 'e':
		dlogging = true;
		break;

	      case 'm':
		 { modopt = true;
		   if ((c = argv[argp][2]) == 0 || argv[argp][3] != 0)
		     quit("'-m' must be followed by single character");
		   else
		   if (ictab[c] == ONE || ictab[c] == MODIFY)
		     set_modifier(c);
		   else
		     quit("character following '-m' is not simple character");
		   break;
		}

	      case 's':
		ringing = false;
		break;

	      case 't':
		tbopt = tbout = true;
		break;

	      case 'u':
		txopt = true; txin = false;
		break;

	      case '\0':
		quit("no option follows '-'");

	      default:
		quit("unknown option %c", argv[argp][1]);
	   }
	else
	 { if (filename[0]==0)
	      cat2(argv[argp],"",filename);
	    else
	      quit("two filenames specified");
	 }

	argp++;
     }

    if (filename[0]==0)
      quit("no file specified");

    if (!txopt)
      txin = !anytail(filename, notxtails);

    if (!tbopt)
      tbout = anytail(filename, tbtails);

    dtos(getpid(), pidname);
    cat3("/tmp/",pidname,".ded",tempname);

    /* make up pathname in pathname array */
    s1 = s2 = pathname;
    s = filename;
    while ((c = *s1++ = *s++) != 0)
      if (c=='/') s2 = s1;

    /* copy rest of name into 'tailname' */
    s = s2; s1 = tailname;
    while((c = *s1++ = *s++) != 0);

    *s2 = 0; /* put end of string after last slash in 'pathname' */

    /* set up temporary file - first set up tty to find out how
     * big screen is
     */
    ttysetup();
    if (stat(filename,&statbuf) == -1)
     { if (errno==ENOTDIR)
	  quit("invalid directory name in %s", filename);
	else
	if (errno==ENOENT)
	 { cat2(pathname, ".", lbuf);
	    if (stat(lbuf,&statbuf) == -1 ||
		      (statbuf.flags & 040000) == 0) /* test for directory */
	      quit("invalid pathname %s", pathname);
	 }

	maketemp(tempname);
    }
    else infile(filename, tempname);

    /* if there aren't enough lines on the screen,
     * set in enough blank lines to fill it up
     */
    while (maxl < LASTINROW)
     { maxl++; l_block[maxl] = l_offset[maxl] = -1; }

    cat2(">","",erow);

    ncat2(tailname, ".dlog", dlogname);
    if (dlogging)
     { if ((tty_input = dlogger = open_dlog(dlogname)) == -1)
	  editerror("can't open .dlog file");
     }
    else
     { tty_input = 2;
	topl = 0;
	set_c(0, 0, &in_c);
	set_c(EDITROW, 1, &edit_c);
	cat2(">", "", erow);
	if ((dlogger = creat_dlog(dlogname)) == -1)
	  editerror("can't create .dlog file");
     }

    setupscreen();

    if (stat(filename,&statbuf) == -1)
      diag("new file - empty screen");
    else
    if (statbuf.nlinks > 1)
      diag("overwriting file with more than one link");

    if (mainloop(ictab, itoe, INMODE))
    /* copy file over to new home  - exit via 'ok' */
     { ttyreset();
	savescreen();

	if (tmp_changed) savefile(putout, false);
	else putout("no changes");
     }
    else
    /* exit via 'q' - don't make any changes to files */
      ttyreset();

    unlink(dlogname); unlink(tempname);

    exit(filewritten);
}

editerror(s,p1,p2,p3,p4,p5,p6)
char *s;
int p1,p2,p3,p4,p5,p6;
 { if (setup_done)
     { position(FOOTROW,0); ttyreset(); }
    putout(s,p1,p2,p3,p4,p5,p6);
    unlink(tempname);
    exit(false);
 }

quit(s,p1,p2,p3,p4,p5,p6)
char *s;
int p1,p2,p3,p4,p5,p6;
 { unlink(dlogname);
    editerror(s, p1, p2, p3, p4, p5, p6);
 }

putout(s, p1,p2,p3,p4,p5, p6)
char *s;
int p1, p2, p3, p4, p5, p6;
 { printf("\n");
    printf(s, p1,p2,p3,p4,p5,p6);
    printf("\n");
 }
