#
/*

	C debugger - variable group handlers

*/

#include <param.h>
#include <user.h>
#include "cdb.h"


gcom(grp)
{
	register struct grpmem *gp1,*gp2;

	if (grp>9 || grp<0) {
		errflg++;
		return;
	}
	while (*lp==' ' || *lp=='\t')
		lp++;
	if (*lp=='\n') {
		dalloc(group[grp].memptr);
		group[grp].memptr = NULL;
		return;
	}
	gp1 = gp2 = NULL;
	recur++;
	while (expr()) {
		if (gp1==NULL)
			gp1 = gp2 = alloc(sizeof membr);
		else {
			gp1->link = alloc(sizeof membr);
			gp1 = gp1->link;
		}
		if (is_auto) {
			gp1->adr = pokeval;
			gp1->offset = symval;
			gp1->fcnnam = fcnstr;
			gp1->fcnnam =- 11;
			gp1->varnam = symptr;
			gp1->varnam =- 12;
		} else
			gp1->offset = gp1->adr = tdot;
		if (*lp=='/') {
			lp++;
			gp1->mode = *lp++;
		} else
			gp1->mode = 'o';
		gp1->mode =| (symflg<<8);
		gp1->link = NULL;
		if (*lp==',')
			lp++;
		is_auto = 0;
	}
	recur = 0;
	if (*lp!='\n' || errflg) {
		errflg++;
		dalloc(gp2);
		return;
	}

	dalloc(group[grp].memptr);
	group[grp].memptr = gp2;
}

grprnt(grp)
{
	extern lastype,lastcom;
	register struct grpmem *ptr;

	printf("\n");
	modifier = 0;
	ptr = group[grp].memptr;
	while (ptr != NULL) {
		symflg = ptr->mode>>8;
		lastype = ptr->mode&0377;
		if (ptr->adr == ptr->offset) {
			dot = ptr->adr;
			psymoff(dot,010000);
			printf("/%c\t",lastype);
		} else {
			printf("%s:%s/%c\t",ptr->fcnnam,ptr->varnam,lastype);
			symval = ptr->offset;
			if (localsym(ptr->adr))
				dot = ssymval;
			else {
				printf("???\n");
				ptr = ptr->link;
				continue;
			}
		}
		switch (lastype) {

		case '!':
		case 'o':
		case 'i':
		case 'd':
		case 'f':
			lastcom = '/';
			break;

		default:
			lastcom = lastype;
		}
		scommand(1);
		ptr = ptr->link;
	}
	printf("\n");
}

dalloc(p)
{
	register struct grpmem *p1,*p2;

	p1 = p;
	if (p1 == NULL)
		return;
	do {
		p2 = p1->link;
		free(p1);
		p1 = p2;
	} while (p2 != NULL);
}
