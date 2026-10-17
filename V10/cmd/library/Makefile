# *************************************************************** 
# *	Copyright (c) 1990 AT&T 				*
# *     All Rights Reserved				        *
# *	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*
# *	The copyright notice above does not			*
# *	evidence any actual or intended				*
# *	publication of such source code.			*
# ***************************************************************
#	@(#)library.mk	1.15  5/5/90 RnD added
# *************************************************************** 
# *	Copyright (c) 1989 AT&T 				*
# *     All Rights Reserved				        *
# *	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*
# *	The copyright notice above does not			*
# *	evidence any actual or intended				*
# *	publication of such source code.			*
# ***************************************************************
# this is the make file for library(1)
# Note that the help messages are gotten from files that are put at
# $INSLIB. If you put them elsewhere change the LIB variable appropriately
#
#########################################
# *** RDS makefile standards applied ***
#########################################
ROOT = 
CC = cc
INC = $(ROOT)/usr/include
MAKE = $(ROOT)/bin/make
INCSYS = $(ROOT)/usr/include/sys
SHELL = /bin/sh
TESTDIR= .
INS = $(ROOT)/etc/install
INSDIR = $(ROOT)/usr/bin
INSLIB = $(ROOT)/usr/lib/library
MAN = $(ROOT)/usr/lman/u_man/man1
LFILES=[2456].* *help* known.list

LDFLAGS= -s $(LDLIBS)
# following is for BSD machines.
# BSD=`if [ -x /bin/sun ]; then if /bin/sun; then echo "-DBSD4_2 -Dstrchr=index -Dstrrchr=rindex"; fi; fi`
BSD=-DBSD4_2
DFLAGS=-DWHERE=\"${INSLIB}\" $(BSD)
CFLAGS=-O -I.  $(DFLAGS)

FILES=library
OFILES=library.o option1.o option3.o reader.o option2.o checkdoc.o gnamef.o uppercase.o

all: $(FILES)

library: $(OFILES)
	$(CC) $(CFLAGS) -o $(TESTDIR)/library $(OFILES) $(LDFLAGS)

lint:
	lint  $(LFLAGS) $(DFLAGS) *.c

install: library
	if test ! -d ${INSLIB} ; \
	then \
	    mkdir ${INSLIB}; \
	    chown bin $(INSLIB); \
	    chgrp bin $(INSLIB); \
	    chmod 775 $(INSLIB); \
	    fi
	for i in $(LFILES); do $(INS) -m 444 -f $(INSLIB) $(TESTDIR)/$$i; done;
	$(INS) -m 555 -f $(INSDIR) $(TESTDIR)/library
	if test  -d $(MAN) ; \
	then \
	    $(INS) -f $(MAN) -m 444 library.1 ; \
	    fi

clean: 
	rm -f *.o

clobber: clean
	rm -f library

