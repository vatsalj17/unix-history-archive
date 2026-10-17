#include "ne.h"

mark(n) int n; {
	if( n )
		printf(".as %d \\k(97\n", n);
	else {
		yyval = oalloc();
		printf(".ds %d \\k(97\n", yyval);
		nrwid(yyval, ps, yyval);
		eht[yyval] = eht[n];
		ebase[yyval] = ebase[n];
	}
	if(dbg)printf(".\tmark %d as %d\n", n, yyval);
}

lineup(n) int n; {
	if( n ) {
		nrwid(n, ps, n);
		printf(".ds %d \\h'|\\n(97u-\\n(%du'\\*(%d\n", n, n, n);
	} else {
		yyval = oalloc();
		printf(".ds %d \\h'|\\n(97u'\n", yyval);
		nrwid(yyval, ps, yyval);
		eht[yyval] = eht[n];
		ebase[yyval] = ebase[n];
	}
	if(dbg)printf(".\tlineup %d in %d\n", n, yyval);
}
