#
#include <ctype.h>
/*
 * NICE(1) command
 */


main(argc,argv)
register int argc;
register char *argv[];
{
	register char *s;
	int lflag = 0;
	int l;
	extern char *_pgmname;
	int pid = -1;

	_pgmname = "nice";
	argc--; argv++;

	if(argc > 0 && **argv == '-'){
		s = (*argv++) + 1;
		argc--;
		if(isdigit(*s) || *s == '-')
			l = atoi(s);
		else {	/* l and p flags */
			if(*s == 'l'){
				lflag++;
				if(!isdigit(s[1]))
					panic("No level number after -l");
				l = atoi(s+1);
			}
			else
				panic("Bad option: '%c'",*s);
			if(argc > 0 && **argv == '-'){
				s = (*argv++) + 1;
				if(*s != 'p')
					panic("Bad option: '%c'",*s);
				if(!isdigit(s[1]))
					panic("No process number after -p");
				pid = atoi(s+1);
				argc--;
			}
		}
	}
	else
		l = 10;	/* default nice number */

	if((lflag == 0 || pid < 0) && argc <= 0)
		panic("Usage: nice [-NUMBER] COMMAND");

	if(lflag){
		if(pid >= 0){
			setblev(pid,l);
			return(0);
		}
		else
			setblev(-1,l);
	}
	else
		nice(l);
	execvp(*argv,argv);
	panic("%#can't execute '%s'",*argv);
}
