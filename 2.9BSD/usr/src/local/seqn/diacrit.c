# include "e.h"
# include "e.def"

diacrit(p1, type) int p1, type; {
	int c, t;

	c = oalloc();
	t = oalloc();
	nrwid(p1, ps, p1);
/*	printf(".nr 10 %du\n", max(eht[p1]-ebase[p1]-VERT(2),0));	/* vertical shift if high */
	printf(".nr 10 %du-1m\n.if \\n(10<0 .nr 10 0\n",eht[p1]-ebase[p1]);
	printf(".if \\n(ct>1 .nr 10 \\n(10+.25m\n");
	printf(".nr %d .1m\n", t);	/* horiz shift if high */
	printf(".if \\n(ct>1 .nr %d .15m\n", t);
	switch(type) {
		case VEC:
			printf(".ds %d \\v'-.4m'\\(->\\v'.4m'\n", c);
			break;
		case DYAD:
			printf(".ds %d \\v'-.4m'\\z\\(<-\\(->\\v'.4m'\n", c);
			break;
		case HAT:
			printf(".ds %d \\v'-.17m'^\\v'.17m'\n", c);
			break;
		case TILDE:
			printf(".ds %d \\v'-.09m'~\\v'.09m'\n", c);
			break;
		case DOT:
			printf(".ds %d \\v'-.67m'.\\v'.67m'\n", c);
			break;
		case DOTDOT:
			printf(".ds %d \\v'-.67m'..\\v'.67m'\n", c);
			break;
		case BAR:
			printf(".ds %d \\\h'.05m'\\l'\\n(%du-.1m\\(rn'\\h'.05m'\n",
/*			printf(".ds %d \\v'.18m'\\h'.05m'\\l'\\n(%du-.1m\\(rn'\\h'.05m'\\v'-.18m'\n",*/
				c, p1);
			break;
		case UNDER:
			printf(".ds %d \\l'\\n(%du\\(ul'\n", c, p1);
			printf(".nr %d 0\n", t);
			printf(".nr 10 0-%d\n", ebase[p1]);
			break;
		}
	nrwid(c, ps, c);
	if (lfont[p1] != ITAL)
		printf(".nr %d 0\n", t);
	printf(".as %d \\h'-\\n(%du-\\n(%du/2u+\\n(%du'\\v'0-\\n(10u'\\*(%d", 
		p1, p1, c, t, c);
	printf("\\v'\\n(10u'\\h'-\\n(%du+\\n(%du/2u-\\n(%du'\n", c, p1, t);
	/* BUG - should go to right end of widest */
	if (type != UNDER)
		eht[p1] += VERT(1);	/* 0.15m */
	if(dbg)printf(".\tdiacrit: %c over S%d, lf=%c, rf=%c, h=%d,b=%d\n",
		type, p1, lfont[p1], rfont[p1], eht[p1], ebase[p1]);
	ofree(c); ofree(t);
}
