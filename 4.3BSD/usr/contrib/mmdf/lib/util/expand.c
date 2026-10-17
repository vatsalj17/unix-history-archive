/*
 *			E X P A N D . C
 *
 *	Expand expands a text string expanding macros provided
 *  in a vector of key and value pairs.
 *
 *  e.g.	expandl(buf, "$A+$B=$(RESULT)", array)
 *  with	char *array[] = {"A", "1",
 *				 "B", "2",
 *				 "RESULT", "3",
 *				 0, 0};
 *  gives	"1+2=3" in buf.
 *
 *  wja@uk.ac.nott.maths		Wed Feb 22 15:00:26 GMT 1984
 *  DPK@BRL, 13 Apr 84	Made more portable and changed array usage.
 */

#include "util.h"

LOCFUN  char *exname(), *exlookup();

char *
expand(buf, fmt, argp)
char *buf, *fmt, **argp;
{
	register char *bp = buf, *cp;
	char name[64];

	while(*fmt) {
		if(*fmt != '$')
			*bp++ = *fmt++;
		else if(*++fmt == '$')
			*bp++ = *fmt++;
		else {
			fmt = exname(name, fmt);
			cp = exlookup(name, argp);
			while(*cp)
				*bp++ = *cp++;
		}
	}
	*bp = '\0';
	return buf;
}

LOCFUN char *
exname(buf, str)
register char *buf, *str;
{
	if(*str == '(') {
		str++;
		while(*str != ')' && *str != '\0')
			*buf++ = *str++;
		if(*str != '\0')
			str++;
	}
	else
		*buf++ = *str++;
	*buf = '\0';
	return str;
}

LOCFUN
char *
exlookup(name, list)
register char *name, **list;
{
	while(*list)
	{
		if(lexequ(name, *(list++)))
			return (*list);
		list++;
	}
	return ("");
}
