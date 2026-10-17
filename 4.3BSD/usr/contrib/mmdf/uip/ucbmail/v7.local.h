/*
 *  V 7 . L O C A L . H 
 *
 *  EE/CIS Computer Lab
 *  Department of Computer and Information Sciences
 *  Department of Electrical Engineering
 *  University of Delaware
 *
 *  REVISION HISTORY:
 *
 *  $Revision: 1.6 $
 *
 *  $Log:	v7.local.h,v $
 * Revision 1.6  86/01/14  16:33:59  galvin
 * Add DAYLIGHT define for use by smtpdate().
 * 
 * Revision 1.5  86/01/05  18:01:36  galvin
 * Add discovered option CANTELL.
 * 
 * Revision 1.4  85/12/18  03:28:31  galvin
 * Undef CANLOCK because we use MMDF locking.
 * 
 * Revision 1.3  85/11/16  14:49:49  galvin
 * Undef SENDMAIL and move a comment to where it belongs.
 * 
 * Revision 1.2  85/11/16  14:46:33  galvin
 * Added comment header for revision history.
 * 
 *
 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)v7.local.h	5.1 (Berkeley) 6/6/85
 */

/*
 * Declarations and constants specific to an installation.
 *
 * Vax/Unix version 7.
 */

#define	GETHOST				/* System has gethostname syscall */
#ifdef	GETHOST
#define	LOCAL		EMPTYID		/* Dynamically determined local host */
#else   GETHOST
#define	LOCAL		'*'		/* Local host id */
#endif	GETHOST

/* fine	MAIL		"/bin/mail"	/* Name of mail sender */
/* fine SENDMAIL	"/usr/lib/sendmail"
					/* Name of classy mail deliverer */
#define	EDITOR		"/usr/ucb/ex"	/* Name of text editor */
#define	VISUAL		"/usr/ucb/vi"	/* Name of display editor */
#define	SHELL		"/bin/csh"	/* Standard shell */
#define	MORE		"/usr/ucb/more"	/* Standard output pager */
#define	HELPFILE	"/usr/lib/Mail.help"
					/* Name of casual help file */
#define	THELPFILE	"/usr/lib/Mail.help.~"
					/* Name of casual tilde help */
#define	POSTAGE		"/usr/adm/maillog"
					/* Where to audit mail sending */
#define	UIDMASK		0177777		/* Significant uid bits */
#define	MASTER		"/usr/lib/Mail.rc"
#define	APPEND				/* New mail goes to end of mailbox */
/* fine CANLOCK				/* Locking protocol actually works */
/* fine	UTIME				/* System implements utime(3C) */
/* UTIMES is preferred if you have it.  */
#define UTIMES                          /* System implements utimes(2) */
#define CANTELL				/* System implements ftell(3S) */
#define DAYLIGHT	1		/* 1 if use daylight, else 0 */

#ifdef V4_2BSD
#ifndef VMUNIX
#define VMUNIX
#endif  VMUNIX
#endif V4_2BSD

#ifdef V4_1BSD
#ifndef VMUNIX
#define	VMUNIX
#endif  VMUNIX
#endif V4_1BSD

#ifndef VMUNIX
#include "./sigretro.h"			/* Retrofit signal defs */
#endif VMUNIX
