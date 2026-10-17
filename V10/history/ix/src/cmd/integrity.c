/* integrity root
 * walks the file tree starting at root, following bottom-label
 * directories and reporting non-bottom contents thereof.
 * Nonbottom directories prune the search.
*/
#include <ftw.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/label.h>

extern struct label labelbot, labelno;

int fn(s, statb, code, S)
char *s;
struct stat *statb;
struct FTW *S;
{
	struct label flab, dlab;
	
	if(getflab(s, &flab) == -1)
		print("U %s\n", s);
	else switch(code) {
		case FTW_D:
			if(strcmp(s, "/dev") == 0
			|| strcmp(s, "/proc") == 0)
				S->quit = FTW_SKD;
			else if(!labEQ(&flab, &labelbot)
			|| flab.lb_fix == F_LOOSE) {
				print("D %s\n", s);
				S->quit = FTW_SKD;
			}
			break;
		case FTW_F:
			if(!labEQ(&flab, &labelbot)
			&& (flab.lb_t|flab.lb_u) == 0)
				print("F %s\n", s);
			break;
	}
	return 0;
}

main(argc, argv)
char **argv;
{
	struct label lab, ceil;
	char *rootdir = argc<2? "/": argv[1];

	print("survey of tree of bottom-label directories\n");
	print("U: unreadable label\n");
	print("D: nonbottom or loose directory\n");
	print("F: nonbottom file\n");
	getplab(&lab, &ceil);
	setplab(&lab, &lab);
	
	if(ftw(rootdir, fn, 20) == -1) {
		perror("integrity");
		return 1;
	}
	return 0;
}
