#include <ctype.h>
#include <sys/label.h>

extern struct label labelbot;
static char frigs[] = "FRC";
static char flags[] = "UYN";
static char privs[] = "guxnlp-";
static int val_frigs[] = { F_FROZEN, F_RIGID, F_CONST };
static int val_flags[] = { L_UNDEF, L_YES, L_NO };
static int val_privs[] = { T_LOG, T_UAREA, T_EXTERN, T_NOCHK,
			   T_SETLIC, T_SETPRIV, 0 };

extern char *strchr();

struct label *
atolab(s)
register char *s;
{
	register int i, c;
	register char *t;
	static struct label y;

	y = labelbot;

	while(*s==' ') s++;
	for( ; *s && (t=strchr(privs, *s)); s++)
		y.lb_t |= val_privs[t-privs];
	while(*s==' ') s++;
	for( ; *s && (t=strchr(privs, *s)); s++)
		y.lb_u |= val_privs[t-privs];
	while(*s) {
		if(t=strchr(flags,*s))
			y.lb_flag = val_flags[t-flags];
		else if(t=strchr(frigs,*s))
			y.lb_fix = val_frigs[t-frigs];
		else if(*s != ' ')
			break;
		s++;
	}

	for(i=0; i<LABSIZ*2 && isxdigit(c=*s); i++) {
		if(isupper(c)) return 0;
		if((c-='0') > 9) c -= ('a'-'0'-10);
		y.lb_bits[i/2] = i%2? y.lb_bits[i/2]|c: c<<4;
		s++;
		while(*s==' ') s++;
	}
	if(*s=='.' && i>0 && i%4==0) {	/* gimmick: labtoa output */
		while(*s=='.') s++;
		for(i/=2; i<LABSIZ; i++)
			y.lb_bits[i] = y.lb_bits[i-2];
	}
	while(*s==' ') s++;
	return *s==0? &y: 0;
}
atopriv(s)
char *s;
{
	register x = 0;
	register char *t;

	for( ; *s && (t=strchr(privs, *s)); s++)
		x |= val_privs[t-privs];
	if (*s)
		for (t=privs, x=0; *t; t++)
			x |= val_privs[t-privs];
	return *s==0? x: ~x;
}
