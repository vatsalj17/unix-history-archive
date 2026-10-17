# @(#)makefile	1.5
#
#	Harvard Lisp Makefile for UCB Installations
#
#	Created by Bob Kridle  -  04-03-81
#
#  NOTE:'make install' will do it all, but do a 'make -n install'
#	first to assure no catastrophies.
#
#	Special versions of macro, linkr, and the system call library
#	are included in ../support to avoid installing macro11
#

TARGET		= /usr/ucb/lisp

#
#	NOTE:	If you want to change this you will also have to change
#		the strings around ln 184 in dstuf.m11
#

LISPLIB		= /usr/lib/lisp

ERRFILE		= lisp.err

OBJFILES	= sysend.obj headr.obj probend.obj early.obj

GLOBALMFILES	= lglob.m11 sparm.m11 macro.m11

SYSENDMFILES	= syss.m11 gcol.m11 now.m11\
		  fp.m11 noroom.m11 sysend.m11

HEADERMFILES	= headr.m11 foodate.m11

PROBENDMFILES	= prob7.m11 prob8.m11 ctable.m11 probend.m11

EARLYMFILES	= dstuf.m11 atdat.m11 subr.m11\
		  subrcom.m11 early.m11

MFILES		= ${GLOBALMFILES} ${SYSENDMFILES} ${HEADERMFILES}\
		  ${PROBENDMFILES} ${EARLYMFILES} filler.m11

SYSLIB		= ../support/sysmac.sml

MACROFLGS = -xs:3

LINKRFLGS = -ln 

MACRO = ../support/macro 

LINKR = ../support/linkr ${LINKRFLGS}

lisp:		preload

l1100.out:	${OBJFILES} ${SYSMACROS}
		${LINKR} ${OBJFILES} -na:$@

sysend.obj:	${SYSENDMFILES} ${GLOBALMFILES}
		${MACRO} ${MACROFLGS} ${GLOBALMFILES} ${SYSENDMFILES} -na:$@

headr.obj:	${HEADERMFILES} ${SYSLIB} ${GLOBALMFILES}
		${MACRO} -ns sparm.m11 ${HEADERMFILES} -na:$@
		rm foodate.m11

probend.obj:	${PROBENDMFILES} ${SYSLIB} ${GLOBALMFILES}
		${MACRO} -xs:2 ${GLOBALMFILES} ${PROBENDMFILES} -na:$@

early.obj:	${EARLYMFILES} ${SYSLIB} ${GLOBALMFILES}
		${MACRO} ${MACROFLGS} ${GLOBALMFILES} ${EARLYMFILES} -na:$@

foodate.m11:
		date | sed "s/.*/	makdate	<&>/" > foodate.m11

.DEFAULT:
		get SCCS/s.$@

clean:
		rm -f ${MFILES} preload.lisp *.map *.obj *.errs *.out code

#
#	Preload built in functions and save runnable copy
#	in the cannonical place
#

preload:	l1100.out preload.lisp
		./l1100.out < preload.lisp
		mv code lisp
#
#	Make errfile and put the rest in the cannonical places
#

install:	l1100.out ${LISPLIB} ${ERRFILE}
		cp lisp ${TARGET}
		chown ucb ${TARGET}
		chmod 0755 ${TARGET}
#
# Put all the good stuff in the right places.
#

${LISPLIB}:
		mkdir ${LISPLIB}
		cp ../lib/* ${LISPLIB}
		chown ucb ${LISPLIB} ${LISPLIB}/*
		chmod 0755 ${LISPLIB}
		chmod 0644 ${LISPLIB}/*

${ERRFILE}:	l1100.out ${LISPLIB}
		./l1100.out -  < /dev/null
		chmod 0644 ${LISPLIB}/${ERRFILE}
