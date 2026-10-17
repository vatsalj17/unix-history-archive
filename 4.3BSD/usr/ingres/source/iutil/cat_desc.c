# include	<ingres.h>
# include	<aux.h>
# include	<access.h>
# include	<sccs.h>

SCCSID(@(#)cat_desc.c	8.1	12/31/84)

/*
**	SYSTEM RELATION DESCRIPTOR CACHE DEFINITION
**
*/

DESC	Reldes;
DESC	Attdes;
DESC	Inddes;
DESC	Treedes;
DESC	Prodes;
DESC	Intdes;


struct desxx	Desxx[] =
{
	"relation",	&Reldes,	&Admin.adreld,
	"attribute",	&Attdes,	&Admin.adattd,
	"indexes",	&Inddes,	NULL,
	"tree",		&Treedes,	NULL,
	"protect",	&Prodes,	NULL,
	"integrities",	&Intdes,	NULL,
	NULL
};
