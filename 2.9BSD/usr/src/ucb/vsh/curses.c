#include "hd.h"
char *BC, *UP, *CM, *CL;  short ospeed;
short PC;
char tspace [512], *atspace;
char *tgetstr ();

curs_init () {

char bp [1024];
char * tgoto ();
char * getenv ();
int putch ();

tgetent (bp, envterm);
atspace = tspace;
PC = tgetflag ("pc", &atspace);
CM=tgetstr ("cm", &atspace);  CL=tgetstr ("cl", &atspace);
UP = tgetstr ("up", &atspace);  BC = tgetstr ("bc", &atspace);
if (CM == CNULL) {
	printf ("Must have a cursor addressable terminal\n");
	leave ();
}
}
