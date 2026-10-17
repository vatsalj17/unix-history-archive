
#
##
#	mwdvi  <->  macwrite to dvi translation program.
#
#	10/11/84  douglas orr


#
#   Installation Options:
#
###
#   version can be  BSD_41
#		or  BSD_42
VERSION=BSD_42

###
#   directory where TeX tfm information can be found
#
TFM_PATH=/usr/lib/font/imagen/tfm


###
#    directory and group for installation of binaries
#
BINDIR	=/usr/local/bin
BINGRP	=bin
BINMODE	=711

INSTALL=install
INSOPTS=-g $(BINGRP) -m $(BINMODE) -s

#
#
CFLAGS= -O

SOURCES	= main.c dvi.c fonts.c md.c
INCLUDES= gen.h option.h md.h dvi.h fonts.h
OBJECTS	= main.o dvi.o fonts.o md.o

mwdvi:	$(OBJECTS)
	cc -o mwdvi $(OBJECTS)

site.h: Makefile
	echo "#define $(VERSION)=1" > site.new.h
	echo "#define TFM_PATH=\"$(TFM_PATH)\"" >> site.new.h
	-if( cmp -s site.h site.new.h );  \
	then \
		$(RM) -f site.new.h; \
	else \
		$(RM) -f site.h; \
		mv site.new.h site.h; \
	fi
		


main.o:		gen.h site.h option.h md.h
md.o:		gen.h site.h option.h md.h dvi.h
dvi.o:		gen.h site.h option.h md.h dvi.h fonts.h
fonts.o:	gen.h site.h option.h md.h dvi.h fonts.h


#
#	administrative stuff
#
install:
	$(INSTALL) $(INSOPTS) mwdvi $(BINDIR)



clean:
	rm $(OBJECTS)

clobber:
	rm -f $(SOURCES) $(INCLUDES) site.h

sources:
	@echo $(SOURCES) $(INCLUDES)

mdump:	mdump.o
	cc -o mdump mdump.o


