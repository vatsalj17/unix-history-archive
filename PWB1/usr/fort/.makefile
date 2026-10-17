
FSRC = /sys/fort

CC = cc
CCFLAGS = -O -s

AS = as

LD = ld
LDFLAGS = -s

MAKE = make
SH = sh -v

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
# Create /usr/fort.
# ------------

MAIN:		fc1 errors
	-rm -f $(DD)*
	-@echo "\n***** Following Files Still Present:";  ls -l $(DD)*;  echo "\n"


fc1:		$(FSRC)/f1/*.s $(FSRC)/f2/*.s
fc1:		$(FSRC)/f3/*.s $(FSRC)/f4/*.s
fc1:		$(FSRC)/fx/*.s $(FSRC)/makefc1.m
fc1:		$(ASDEP)
	chdir $(FSRC); $(MAKE) -f makefc1.m fc1
	-mv fc1 $(DD)fc1
	mv $(FSRC)/fc1 fc1
	-chdir $(FSRC); $(MAKE) -f makefc1.m clean
	strip fc1
	chmod 755 fc1
	-chown bin fc1
	ls -l fc1
	-rm -f $(DD)*

errors:
	echo '\n*** "errors" file missing. ***\n'

RC:	# dummy name to force re-compiling things.
