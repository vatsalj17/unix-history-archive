#include "hd.h"
#include "mydir.h"

dircmd (cmd) register cmd; {

	register ret;		/* return value */

	ret = REPLOT;

	if (cmd == EOT) leave ();
	else if (cmd == LF)
		ret = enterdir (DOTDOT);
	else if ((cmd == '+') || (cmd == ';') || (cmd == 'u'))
		cpage = ((cpage >= tpages) ? 1 : cpage + 1);
	else if (cmd >= 'a' && cmd <= 'z') {
		cmd -= 'a';
		if (cmd >= pageend) ret = NOREPLOT;
		else if (enterfile (filename (cmd)) == FAILURE) {
			longfile (cmd);  ret = NOREPLOT;
		}

	} else if (cmd > '0' && cmd <= '9')
		cpage = cmd - '0';
	else if (cmd == '0') cpage = 10;
	else if (cmd == '-') {
		cpage--;
		if (cpage < 1) cpage = tpages;
	}
	else ret = NOOP;
	return ret;
}
