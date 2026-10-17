/*
 * Error-handling for an application.
 */

/*
 *-IMPORTS:
 *	<sys/compat.h>
 *		CONST
 *		USE_PROTO
 *		ARGS ()
 *	<setjmp.h>
 *		longjmp ()
 *	<stdarg.h>
 *		va_list
 *		va_start ()
 *		va_end ()
 *	<stdio.h>
 *		stderr
 *		fprintf ()
 *		putc ()
 *		vfprintf ()
 *	<stdlib.h>
 *		EXIT_FAILURE
 *		exit ()
 */

#include <sys/compat.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ehand.h"


/*
 * Some ancient systems do not define EXIT_SUCCESS or EXIT_FAILURE.
 */

#ifndef	EXIT_FAILURE
#define	EXIT_FAILURE	1
#endif


ehand_t	      *	estack_base;
CONST char    *	command_name;

/*
 * Register the name of the currently executing command with the error-
 * reporting system.
 */

#if	USE_PROTO
void (register_name) (CONST char * name)
#else
void
register_name ARGS ((name))
CONST char    *	name;
#endif
{
	command_name = name;
}


/*
 * Report an error, but do not throw to a handler.
 */

#if	USE_PROTO
void (report_error) (CONST char * fmt, ...)
#else
void
report_error ARGS ((fmt))
CONST char    *	fmt;
#endif
{
	va_list		args;

	va_start (args, fmt);
	if (command_name != NULL)
		(void) fprintf (stderr, "%s: ", command_name);
	(void) fprintf (stderr, "error: ");
	(void) vfprintf (stderr, fmt, args);
	va_end (args);

	putc ('\n', stderr);
}


/*
 * Throw an error - write out an error report and throw to the last installed
 * handler (or call exit () if there is no installed handler).
 */

#if	USE_PROTO
NO_RETURN (throw_error) (int ecode, CONST char * fmt, ...)
#else
NO_RETURN
throw_error ARGS ((ecode, fmt))
int		ecode;
CONST char    *	fmt;
#endif
{
	va_list		args;

 	va_start (args, fmt);
 	if (command_name != NULL)
 		(void) fprintf (stderr, "%s: ", command_name);
	(void) fprintf (stderr, "error: ");
	(void) vfprintf (stderr, fmt, args);
	va_end (args);

	putc ('\n', stderr);

	if (estack_base == NULL)
		exit (ecode == 0 ? EXIT_FAILURE : ecode);

	estack_base->eh_code = ecode;
	longjmp (estack_base->eh_buf, -1);
}


/*
 * This function can be called when the code to pop the current exception
 * handler detects that the stack base is not where it should be (implying
 * that an inner function did not pop its handler).
 */

#if	USE_PROTO
void (error_error) (CONST char * file, int line)
#else
void
error_error ARGS ((file, line))
CONST char    *	file;
int		line;
#endif
{
	if (command_name != NULL)
		(void) fprintf (stderr, "%s: ", command_name);

	(void) fprintf (stderr, "fatal error : Exception handler nesting error "
				"detected in file\n  '%s' at line %d\n",
			file, line);

	exit (EXIT_FAILURE);
}


/*
 * Chain to the next error-handler in the stack of error handlers after the
 * handler passed to us.
 */

#if	USE_PROTO
void (chain_error) (ehand_t * err)
#else
void
chain_error ARGS ((err))
ehand_t	      *	err;
#endif
{
	if (err != estack_base)
		error_error ("chain_error, " __FILE__, __LINE__);

	if ((estack_base = err->eh_next) == NULL)
		exit (err->eh_code == 0 ? EXIT_FAILURE : err->eh_code);

	estack_base->eh_code = err->eh_code;
	longjmp (estack_base->eh_buf, -1);
}

