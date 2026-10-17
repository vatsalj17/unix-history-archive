/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1993 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */

#ifndef	__COMMON_FEATURE_H__
#define	__COMMON_FEATURE_H__

/*
 * This header file deals with some minor complications that exist in the
 * world of feature-tests. Standard style for new code is to use the simpler
 * and more powerful #if test rather than the cumbersome and less useful
 * #ifdef/#ifndef tests that untutored programmers gravitate to.
 *
 * However, this is made more difficult by historical problems, and some
 * latitude extended to users. For example, while POSIX.1 introduces a
 * feature-test macro called _POSIX_SOURCE, no value for this macro was
 * recommended, and so users typically #define this symbol to have no value,
 * making the #if form of feature-test more complex. Conversely, the ISO C
 * standard mandates a value for __STDC__, but users have uniformly ignored
 * this fact.
 *
 * This file mutates definitions of well-known feature tests so that the #if
 * form of test can be used universally.
 */

#define	__EMPTY(x)		((1 - x - 1) == 2)

#define	__UNDEFINED_OR_EMPTY(x)	(! defined (x) || __EMPTY (x))


/*
 * Since GNU C is kind enough to supply a version number, here we define a
 * macro that can be used to test to see if the compiler is a version of GCC
 * after a specific major/minor version.
 */

/*
 * Older GCC versions did not define a minor number.
 */
#if !defined(__GNUC_MINOR__) && __GNUC__
#define __GNUC_MINOR__ 3
#endif

#define	__IS_GCC_FROM(major, minor) \
			(__GNUC__ > (major) || \
			 (__GNUC__ == (major) && __GNUC_MINOR__ >= (minor)))

/*
 * Command-line users frequently misapprehend the usage style of many
 * feature-tests; here we try and deal with that.
 */

#if	__EMPTY (_SYSV3)
# error	If you define the "_SYSV3" feature-test macro, you must give it a value
#endif

#if	__EMPTY (_SYSV4)
# error	If you define the "_SYSV4" feature-test macro, you must give it a value
#endif

#if	__EMPTY (_POSIX_C_SOURCE)
# error	If you define the "_POSIX_C_SOURCE" feature-test macro, you must give it a value
#endif

#if	__EMPTY (_DDI_DKI)
# error	If you define the "_DDI_DKI" feature-test macro, you must give it a value
#endif

#if	__EMPTY (_SUPPRESS_BSD_DEFINITIONS)
# error	If you define the "_SUPPRESS_BSD_DEFINITIONS" feature-test macro, you must give it a value
#endif


/*
 * The POSIX.2 standard introduces a new feature-test symbol, _POSIX_C_SOURCE.
 * When given the value 1 or 2, the effect is the same as if _POSIX_SOURCE had
 * been defined as specified in the POSIX.1 standard. When given the value 2,
 * definitions mandanted by the POSIX.2 standard shall also be made visible.
 *
 * Below, we set things up so that headers can uniformly use _POSIX_C_SOURCE
 * as a feature-test without worrying about the nasty _POSIX_SOURCE problems.
 */

#if	defined (_POSIX_SOURCE) && ! _POSIX_C_SOURCE
# define	_POSIX_C_SOURCE	1
#endif

#if	_POSIX_C_SOURCE && ! defined (_POSIX_SOURCE)
# define	_POSIX_SOURCE	1
#endif


/*
 * We have a feature-test _STDC_SOURCE analagous to _POSIX_SOURCE which
 * selects a minimal ISO C compilation environment. This is mutually exclusive
 * with _POSIX_SOURCE. We could either complain or do something tricky like
 * select the one with the highest value.
 */

#if	_STDC_SOUCE || _POSIX_C_SOURCE
# if	_STDC_SOURCE > _POSIX_C_SOURCE

#  undef	_POSIX_C_SOURCE

# elif	_STDC_SOURCE < _POSIX_C_SOURCE

#  undef	_STDC_SOURCE

# else

#  error You cannot select both _POSIX_SOURCE and _STDC_SOURCE in a compilation

# endif
#endif


/*
 * System V Drivers typically use -D_KERNEL to activate driver-specific stuff
 * in headers. Since the only SVR4 kernel code we support is DDI/DKI, we make
 * the DDI/DKI stuff visible. Defining both _KERNEL and _DDI_DKI is senseless,
 * but we permit it anyway.
 */

#if	__EMPTY (_KERNEL)
# undef	_KERNEL

# if	! _DDI_DKI
#  define	_DDI_DKI	1
# endif

#endif


/*
 * For Coherent: avoid the use of COHERENT as a feature-test, use __COHERENT__
 * instead. Using COHERENT as a feature-test is not recommended as a future
 * release of the Coherent tools will not supply it.
 *
 * Note that if this *is* Coherent, then we are part of version 4.2 at the
 * earliest.
 */

#if	defined (KERNEL) || defined (__KERNEL__)

#undef	KERNEL
#undef	__KERNEL__
#define	_KERNEL		1

#endif

#ifdef	COHERENT
#define	__COHERENT__	1
/* #undef COHERENT */
#endif


#if	__COHERENT__ == 1
#undef	__COHERENT__
#define	__COHERENT__	0x0420
#endif


/*
 * The MWC port of GCC defines the _I386 and _IEEE macros through the specs
 * file, so that when -ansi is used the definitions are suppressed. This is a
 * little silly, since the definitions are in the implementation namespace,
 * so we fix this.
 */

#if	__GNUC__ && defined (___I386__) && ! defined (_I386)
# define	_I386	1
#endif
#if	__GNUC__ && defined (___IEEE__) && ! defined (_IEEE)
# define	_IEEE	1
#endif

#endif	/* ! defined (__COMMON_FEATURE_H__) */
