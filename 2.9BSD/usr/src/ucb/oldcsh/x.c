char	xstr[];
#line 1 "sh.init.c"



#line 1 "./sh.local.h"


















#line 23 "./sh.local.h"

#line 26 "./sh.local.h"



#line 31 "./sh.local.h"

































#line 74 "./sh.local.h"


#line 86 "./sh.local.h"


#line 90 "./sh.local.h"

#line 4 "sh.init.c"




extern	int await();
extern	int chngd();
extern	int doalias();
extern	int dobreak();
extern	int docontin();
extern	int doecho();
extern	int doelse();
extern	int doend();
extern	int doendif();
extern	int doendsw();
extern	int doexit();
extern	int doforeach();
extern	int doglob();
extern	int dogoto();
extern	int dohash();
extern	int dohist();
extern	int doif();
extern	int dolet();
extern	int dologin();
extern	int dologout();
extern	int donewgrp();
extern	int donice();
extern	int donohup();
extern	int doonintr();
extern	int dorepeat();
extern	int doset();

extern	int dosetenv();

extern	int dosource();
extern	int doswbrk();
extern	int doswitch();
extern	int dotime();

extern	int doumask();

extern	int dowhile();
extern	int dozip();
extern	int execash();
#line 49 "sh.init.c"

extern	int goodbye();
extern	int shift();
extern	int showall();
extern	int unalias();
extern	int dounhash();
extern	int unset();



struct	biltins {
	char	*bname;
	int	(*bfunct)();
	short	minargs, maxargs;
} bfunc[] = {
	(&xstr[0]),		dolet,		0,	1000,
	(&xstr[2]),	doalias,	0,	1000,
#line 68 "sh.init.c"

	(&xstr[8]),	dobreak,	0,	0,
	(&xstr[14]),	doswbrk,	0,	0,
	(&xstr[22]),		dozip,		0,	1,
	(&xstr[27]),		chngd,		0,	1,
	(&xstr[30]),	chngd,		0,	1,
	(&xstr[36]),	docontin,	0,	0,
	(&xstr[45]),	dozip,		0,	0,
	(&xstr[53]),		doecho,		0,	1000,
	(&xstr[58]),		doelse,		0,	1000,
	(&xstr[63]),		doend,		0,	0,
	(&xstr[67]),	dozip,		0,	0,
	(&xstr[73]),	dozip,		0,	0,
	(&xstr[79]),		execash,	1,	1000,
	(&xstr[84]),		doexit,		0,	1000,
	(&xstr[89]),	doforeach,	3,	1000,
	(&xstr[97]),		doglob,		0,	1000,
	(&xstr[102]),		dogoto,		1,	1,
#line 88 "sh.init.c"

	(&xstr[107]),	dohist,		0,	0,
	(&xstr[70]),		doif,		1,	1000,
	(&xstr[115]),	dologin,	0,	1,
	(&xstr[121]),	dologout,	0,	0,
	(&xstr[128]),	donewgrp,	1,	1,
	(&xstr[135]),		donice,		0,	1000,
	(&xstr[140]),	donohup,	0,	1000,
	(&xstr[146]),	doonintr,	0,	2,
	(&xstr[153]),	dohash,		0,	0,
	(&xstr[160]),	dorepeat,	2,	1000,
	(&xstr[167]),		doset,		0,	1000,

	(&xstr[171]),	dosetenv,	2,	2,

	(&xstr[178]),	shift,		0,	1,
	(&xstr[184]),	dosource,	1,	1,
	(&xstr[191]),	doswitch,	1,	1000,
	(&xstr[198]),		dotime,		0,	1000,

	(&xstr[203]),	doumask,	0,	1,

	(&xstr[209]),	unalias,	1,	1000,
	(&xstr[217]),	dounhash,		0,	0,
	(&xstr[224]),	unset,		1,	1000,
	(&xstr[230]),		await,		0,	0,
	(&xstr[235]),	dowhile,	1,	1000,
	0,		0,		0,	0,
};





















struct srch {
	char	*s_name;
	short	s_value;
} srchn[] = {
	(&xstr[0]),			13,
	(&xstr[8]),		0,
	(&xstr[14]),		1,
	(&xstr[22]),			2,
	(&xstr[45]), 		3,
	(&xstr[58]),			4,
	(&xstr[63]),			5,
	(&xstr[67]),		6,
	(&xstr[73]),		7,
	(&xstr[84]),			8,
	(&xstr[89]), 	9,
	(&xstr[102]),			10,
	(&xstr[70]),			11,
	(&xstr[241]),		12,
	(&xstr[226]),			14,
	(&xstr[191]),		15,
	(&xstr[235]),		18,
	0,		0,
};

char	*mesg[] = {
	0,
	(&xstr[247]),
	0,
	(&xstr[254]),
	(&xstr[259]),
	(&xstr[279]),
	(&xstr[294]),
	(&xstr[303]),
	(&xstr[312]),
	(&xstr[331]),
	(&xstr[338]),
	(&xstr[348]),
	(&xstr[371]),
	0,
	(&xstr[387]),
	(&xstr[399]),
};
