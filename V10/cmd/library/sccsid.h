/*	headers:1.16:sccsid.h	1.1	*/
/*
 *	VERSION() is used to identify a C module's sccs id.
 *		(the number in parens is the sccs id of the
 *		"headers" component of this delivery).
 */
#ifdef lint
#define VERSION(x);
#else
#define VERSION(x) static char _sccsid[]="x(headers:1.16)";
#endif

/*
 *		NOTE: sccsid.h needs to be admin'd whenever headers
 *		get turned over.  To do this 
 *
 *		sadmin -fq`sprs -d:I: headers components` headers sccsid.h"
 *
 */
