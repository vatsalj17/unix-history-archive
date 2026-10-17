# ***************************************************************
# *	Copyright (c) 1987 AT&T 				*
# *     All Rights Reserved				        *
# *	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*
# *	The copyright notice above does not			*
# *	evidence any actual or intended				*
# *	publication of such source code.			*
# ***************************************************************
# @(#)makefile	1.1.1.12	(6/5/87)

########
#
#	master 630 MTG makefile
#
########

########
#
#	All decendent makefiles support the following standard targets
#
#	build		builds all the products specified by PRODUCTS.
#			This is the default for all makefiles.
#	install		installs products
#	clean		removes all temporary files (ex. installable object)
#	clobber		"cleans", and then removes $(PRODUCTS)
#	tape		creates a new directory hierarchy for the app dev pack
#			and copies all source files to that new dir.
#
########

########
#
#	Macro definitions
#
#	TAPEROOT is used when the tape target is specified.  It is
#	the pathname for the location of the new directory hierarchy.
#
#	MODE775 is used by descendent makefiles to set the file mode
#	for products installed in the $DMD/bin directory.
#
#	MODE664 is used by descendent makefiles to set the file mode
#	for products installed in the $DMD/lib directory.
#
########
MAKE_CMD=make
ACTION=build
TAPEROOT=
MODE775=775
MODE664=644

########
#
# In order for the macro definitions specified on the
# command line to be passed to decendent makefiles
# they must be included in the following generic macro definition.
#
########
MACROS=MODE775=$(MODE775) MODE664=$(MODE664) TAPEROOT=$(TAPEROOT) \
	ACTION=$(ACTION) MAKE_CMD=$(MAKE_CMD)

########
#
# WARNING: order of the following macro definitions is important!
#
########

DMDCC=src/dmdcc

MC68SGS=src/ccs

COREPRODUCTS = \
	src/lib/libj \
	src/lib/startup \
	src/lib/libjx \
	src/lib/libfw \
	src/demo \
	src/dmdcat \
	src/dmdld \
	src/dmdman \
	src/dmdversion \
	src/icon \
	src/jx \
	src/jim \
	src/loadfont \
	src/dmdmemory \
	src/dmdpi \
	src/ucache \
	src/wtinit


all:	v9support $(DMDCC) $(MC68SGS) $(COREPRODUCTS)

v9support:
	cd src/ccs/cmd/sgs/libld/mc68; make
	cd 630mux; make

copyright:
	@echo "\n\n**********************************************"
	@echo "*         Copyright (c) 1986 AT&T            *"
	@echo "*           All Rights Reserved              *"
	@echo "**********************************************\n\n"

readme:	copyright 
	@cat README

tape: tape1
tape: tape2
tape: tape3
tape: tape4

tape1:	makedir
tape2:	cpmisc

tape3:	Tape3
Tape3:
	@echo "\nmaking $(ACTION) for src/ccs"; \
	cd src/ccs; exec $(MAKE_CMD) $(MACROS) ACTION=$(ACTION) $(ACTION)

tape4:	$(DMDCC) $(COREPRODUCTS)

########
#
#	Targets for making descendant directories
#
########

src/ccs:	Dsrc/ccs
Dsrc/ccs:
	@echo "\nmaking $(ACTION) for src/ccs"; \
	cd src/ccs; exec $(MAKE_CMD) $(MACROS) ACTION=$(ACTION) all libs

src/dmdcc:	Dsrc/dmdcc
Dsrc/dmdcc:
	@echo "\nmaking $(ACTION) for src/dmdcc"; \
	cd src/dmdcc; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/lib/libj:	Dsrc/lib/libj
Dsrc/lib/libj:
	@echo "\nmaking $(ACTION) for src/lib/libj"; \
	cd src/lib/libj; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/lib/startup:	Dsrc/lib/startup
Dsrc/lib/startup:
	@echo "\nmaking $(ACTION) for src/lib/startup"; \
	cd src/lib/startup; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/lib/libjx:	Dsrc/lib/libjx
Dsrc/lib/libjx:
	@echo "\nmaking $(ACTION) for src/lib/libjx"; \
	cd src/lib/libjx; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/lib/libfw:	Dsrc/lib/libfw
Dsrc/lib/libfw:
	@echo "\nmaking $(ACTION) for src/lib/libfw"; \
	cd src/lib/libfw; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/demo:	Dsrc/demo
Dsrc/demo:
	@echo "\nmaking $(ACTION) for src/demo"; \
	cd src/demo; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/dmdcat:	Dsrc/dmdcat
Dsrc/dmdcat:
	@echo "\nmaking $(ACTION) for src/dmdcat"; \
	cd src/dmdcat; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/dmdld:	Dsrc/dmdld
Dsrc/dmdld:
	@echo "\nmaking $(ACTION) for src/dmdld"; \
	cd src/dmdld; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/dmdman:	Dsrc/dmdman
Dsrc/dmdman:
	@echo "\nmaking $(ACTION) for src/dmdman"; \
	cd src/dmdman; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/dmdversion:	Dsrc/dmdversion
Dsrc/dmdversion:
	@echo "\nmaking $(ACTION) for src/dmdversion"; \
	cd src/dmdversion; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/icon:	Dsrc/icon
Dsrc/icon:
	@echo "\nmaking $(ACTION) for src/icon"; \
	cd src/icon; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/jim:	Dsrc/jim
Dsrc/jim:
	@echo "\nmaking $(ACTION) for src/jim"; \
	cd src/jim; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/jx:	Dsrc/jx
Dsrc/jx:
	@echo "\nmaking $(ACTION) for src/jx"; \
	cd src/jx; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/loadfont:	Dsrc/loadfont
Dsrc/loadfont:
	@echo "\nmaking $(ACTION) for src/loadfont"; \
	cd src/loadfont; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/dmdmemory:	Dsrc/dmdmemory
Dsrc/dmdmemory:
	@echo "\nmaking $(ACTION) for src/dmdmemory"; \
	cd src/dmdmemory; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/dmdpi:	Dsrc/dmdpi
Dsrc/dmdpi:
	@echo "\nmaking $(ACTION) for src/dmdpi"; \
	cd src/dmdpi; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/ucache:	Dsrc/ucache
Dsrc/ucache:
	@echo "\nmaking $(ACTION) for src/ucache"; \
	cd src/ucache; exec $(MAKE_CMD) $(MACROS) $(ACTION)

src/wtinit:	Dsrc/wtinit
Dsrc/wtinit:
	@echo "\nmaking $(ACTION) for src/wtinit"; \
	cd src/wtinit; exec $(MAKE_CMD) $(MACROS) $(ACTION)


########
#
#	create the directory hierarchy under TAPEROOT
#
########

makedir:
	@if [ $(TAPEROOT) = `pwd` ]; \
	then \
		echo "TAPEROOT cannot equal current directory"; \
		exit 1; \
	fi; \
	if [ ! -d $(TAPEROOT) ]; \
	then \
		echo "\tmkdir $(TAPEROOT)"; \
		mkdir $(TAPEROOT); \
	fi
	@cd $(TAPEROOT); \
	if [ ! -d ./man ]; \
	then \
		mkdir ./man; \
		mkdir ./man/src; \
		mkdir ./man/src/p_man; \
		mkdir ./man/src/p_man/man3; \
		mkdir ./man/src/p_man/man4; \
		mkdir ./man/src/p_man/man5; \
		mkdir ./man/src/u_man; \
		mkdir ./man/src/u_man/man1; \
		mkdir ./man/u_man; \
		mkdir ./man/u_man/man1; \
		mkdir ./man/p_man; \
		mkdir ./man/p_man/man3; \
		mkdir ./man/p_man/man4; \
		mkdir ./man/p_man/man5; \
	fi; \
	if [ ! -d ./bin ]; \
	then \
		mkdir ./bin; \
	fi; \
	if [ ! -d ./lib ]; \
	then \
		mkdir ./lib; \
		mkdir ./lib/demobin; \
		mkdir ./lib/demolib; \
		mkdir ./lib/layersys; \
	fi; \
	if [ ! -d ./examples ]; \
	then \
		mkdir ./examples; \
		mkdir ./examples/Caching; \
		mkdir ./examples/Fonts; \
		mkdir ./examples/GettingStarted; \
		mkdir ./examples/Graphics; \
		mkdir ./examples/Jx; \
		mkdir ./examples/Keyboard; \
		mkdir ./examples/Messages; \
		mkdir ./examples/Resources; \
		mkdir ./examples/UserInterface; \

	fi; \
	if [ ! -d ./termfonts ]; \
	then \
		mkdir ./termfonts; \
	fi; \
	if [ ! -d ./icons ]; \
	then \
		mkdir ./icons; \
		mkdir ./icons/16x16; \
		mkdir ./icons/face48; \
		mkdir ./icons/large; \
		mkdir ./icons/texture; \
	fi
	@cd $(TAPEROOT); \
	if [ ! -d ./src ]; \
	then \
		mkdir ./src; \
		mkdir ./src/ccs; \
		mkdir ./src/ccs/cmd; \
		mkdir ./src/ccs/cmd/sgs; \
		mkdir ./src/ccs/lib; \
		mkdir ./src/lib; \
		mkdir ./src/jim; \
		mkdir ./src/dmdpi; \
		mkdir ./src/wtinit; \
	fi
	@cd $(TAPEROOT); \
	if [ ! -d ./include ]; \
	then \
		mkdir ./include; \
		mkdir ./include/sys; \
	fi; \
	if [ ! -d ./terminfo ]; \
	then \
		mkdir ./terminfo; \
	fi

########
#
#	Copy the miscellaneous source files that are not taken care
#	of by descendent makefiles.
#
########

cpmisc:
	@cpio -pdv $(TAPEROOT) < ./SOURCE

	@cd src; \
	cpio  -pdv $(TAPEROOT)/src < ./SOURCE

	@cd examples; \
	cpio  -pdv $(TAPEROOT)/examples < ./SOURCE

	@cd examples/Caching; \
	cpio  -pdv $(TAPEROOT)/examples/Caching < ./SOURCE

	@cd examples/Fonts; \
	cpio  -pdv $(TAPEROOT)/examples/Fonts < ./SOURCE

	@cd examples/GettingStarted; \
	cpio  -pdv $(TAPEROOT)/examples/GettingStarted < ./SOURCE

	@cd examples/Graphics; \
	cpio  -pdv $(TAPEROOT)/examples/Graphics < ./SOURCE

	@cd examples/Jx; \
	cpio  -pdv $(TAPEROOT)/examples/Jx < ./SOURCE

	@cd examples/Keyboard; \
	cpio  -pdv $(TAPEROOT)/examples/Keyboard < ./SOURCE

	@cd examples/Messages; \
	cpio  -pdv $(TAPEROOT)/examples/Messages < ./SOURCE

	@cd examples/Resources; \
	cpio  -pdv $(TAPEROOT)/examples/Resources < ./SOURCE

	@cd examples/UserInterface; \
	cpio  -pdv $(TAPEROOT)/examples/UserInterface < ./SOURCE

	@cd termfonts; \
	cpio  -pdv $(TAPEROOT)/termfonts < ./SOURCE

	@cd icons/16x16; \
	cpio  -pdv $(TAPEROOT)/icons/16x16 < ./SOURCE

	@cd icons/face48; \
	cpio  -pdv $(TAPEROOT)/icons/face48 < ./SOURCE

	@cd icons/large; \
	cpio  -pdv $(TAPEROOT)/icons/large < ./SOURCE

	@cd icons/texture; \
	cpio  -pdv $(TAPEROOT)/icons/texture < ./SOURCE

	@cd include; \
	cpio  -pdv $(TAPEROOT)/include < ./SOURCE

	@cd include/sys; \
	cpio  -pdv $(TAPEROOT)/include/sys < ./SOURCE

	@cd $(TAPEROOT)/lib/layersys; \
	>"lsys.8;8;?"; \
	>"lsys.8;8;5"; \
	>"lsys.8;8;6"
	
	@cd man/src; \
	cpio  -pdv $(TAPEROOT)/man/src < ./SOURCE

	@cd man/src/p_man/man3; \
	cpio  -pdv $(TAPEROOT)/man/src/p_man/man3 < ./SOURCE

	@cd man/src/p_man/man4; \
	cpio  -pdv $(TAPEROOT)/man/src/p_man/man4 < ./SOURCE
	
	@cd man/src/p_man/man5; \
	cpio  -pdv $(TAPEROOT)/man/src/p_man/man5 < ./SOURCE

	@cd man/src/u_man/man1; \
	cpio  -pdv $(TAPEROOT)/man/src/u_man/man1 < ./SOURCE

	@cd terminfo; \
	cpio  -pdv $(TAPEROOT)/terminfo < ./SOURCE
