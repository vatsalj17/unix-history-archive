#	For 2 lines, set NHASP=2.  For 1 line, set NHASP=.
NHASP =

#		$(USR) is target file system for hasp & rje directories.
SRC = /sys/source
USR = /usr

CC = cc
CCFLAGS = -O -s

AS = as

LD = ld
LDFLAGS = -s

SH = sh -v
MAKE = make

DD = TMP

#	Dependencies: set name to RC to force recomp.
CTIME =		# for ctime(III).
CCDEP =		# for C compiler, CC.
ASDEP =		# for assembler.
YACCDEP=	# for yacc(I).
LPWDEP =	# for -lPW lib.
LSDEP =		# for -lS lib.
LPDEP =		# for -lp lib.
LpwDEP =	# for -lpw lib.


# ------------
#  Create /usr/hasp.
# ------------

HASPFILES = compact haspcopy haspdisp hasphalt haspinit haspmain \
		haspqer hasprecv haspxmit \
		lsl ptran ptrans scan \
		shqer snoop0 snoop1 testjob usage

MAIN:		info job pool \
		$(USR)/rje $(USR)/rje/sque $(USR)/rje/lines $(USR)/rje/sys \
		HASP$(NHASP)
	echo "\n*** DONE! ***\n"


HASP:		$(HASPFILES)	# Create hasp crud here.

$(HASPFILES):		$(SRC)/rje/hasp/*.[chs] $(SRC)/rje/hasp/*.rc
$(HASPFILES):		$(SRC)/rje/hasp/lsl $(SRC)/rje/hasp/ptran
$(HASPFILES):		$(SRC)/rje/hasp/ptrans $(SRC)/rje/hasp/scan
$(HASPFILES):		$(SRC)/rje/hasp/testjob
$(HASPFILES):		$(CCDEP) $(ASDEP) $(LPWDEP) $(CTIME)
	@echo "Remember: you must be root or super-user!"
	chdir $(SRC)/rje/hasp; $(SH) hasp.rc "$(USR)" "$(CC)"

info:
	mkdir info
	chmod 755 info
	-chown rje info

job:
	mkdir job
	chmod 777 job
	-chown rje job

pool:
	mkdir pool
	chmod 777 pool
	-chown rje pool

$(USR)/rje/sque:
	mkdir $(USR)/rje/sque
	chmod 755 $(USR)/rje/sque
	-chown rje $(USR)/rje/sque

$(USR)/rje/sys:		$(SRC)/rje/hasp/sys
	cp $(SRC)/rje/hasp/sys $(USR)/rje/sys
	-chown rje $(USR)/rje/sys

$(USR)/rje/lines:	RC	# ALWAYS copy appropriate lines file.
	cp $(SRC)/rje/hasp/lines$(NHASP) $(USR)/rje/lines
	-chown rje $(USR)/rje/lines

$(USR)/rje:
	mkdir $(USR)/rje
	chmod 555 $(USR)/rje
	-chown rje $(USR)/rje

# Create links in hasp2 after creating hasp crud here.
HASP2:		HASP $(USR)/hasp2 $(USR)/hasp2/.makefile
	echo "Make links in $(USR)/hasp2 for second line."
	chdir $(USR)/hasp2; $(MAKE) -f .makefile "USR=$(USR)"

$(USR)/hasp2/.makefile:		makehasp2
	cp makehasp2 $(USR)/hasp2/.makefile
	-chown rje $(USR)/hasp2/.makefile

$(USR)/hasp2:
	mkdir $(USR)/hasp2
	-chown rje $(USR)/hasp2


RC:	# dummy name to force re-compiling things.
