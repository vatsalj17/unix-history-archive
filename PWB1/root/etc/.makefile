
SRC = /sys/source
SYSINC = /sys/sys/hd

CC = cc
CCFLAGS = -O -s

AS = as

LD = ld
LDFLAGS = -s

DD = TMP
STICKY = OLD

MAKE = make
SH = sh -v

#	Dependencies: set name to RC to force recomp.
CTIME =		# for ctime(III).
CCDEP =		# for C compiler, CC.
ASDEP =		# for assembler.
YACCDEP=	# for yacc(I).
LPWDEP =	# for -lPW lib.
LSDEP =		# for -lS lib.
LPDEP =		# for -lp lib.
LpwDEP =	# for -lpw lib.
SYSDEP=		# for $(SYSINC) system include files.


# ------------
#  Create /etc
# ------------
MAIN:		ac bcopy cache check \
		clri clrm config cron cu \
		dcheck devnm dump fcheck fsdb getty glob \
		icheck init labelit lastcom log mkfs mknod mount ncheck \
		restor rmall sa setmnt setuid sysfix umount unlink \
		vpd vc10 vc50 vc88 wall wunt

#		Text/data files used by specific things in /etc.
MAIN:		checklist master mnttab

#		Shells & other files -- just check that they exist.
MAIN:		mkrp03 mkrp04 rc shutdown startup whodo
MAIN:		filesave.x getoff group motd passwd sha ttys utmp

MAIN:
	-rm -f $(DD)*
	-@echo "\n***** Following Files Still Present:";  ls -l $(DD)*;  echo "\n"
	-@echo "\n***** Following Sticky Files Still Present, Remove After Next Reboot:"; \
			ls -l $(STICKY)*;  echo "\n"

ac:		$(SRC)/acctg/cacct.c $(SRC)/acctg/cvtlid.c \
		$(CCDEP) $(CTIME)
	$(CC) $(CCFLAGS) $(SRC)/acctg/cacct.c $(SRC)/acctg/cvtlid.c
	-rm cacct.o cvtlid.o
	chmod 755 a.out
	-chown root a.out
	-mv ac $(DD)ac
	mv a.out ac
	ls -l ac
	-rm -f $(DD)*

bcopy:		$(SRC)/s8/bcopy.c $(CCDEP)
	$(CC) $(CCFLAGS) -n $(SRC)/s8/bcopy.c
	chmod 755 a.out
	-chown root a.out
	-mv bcopy $(DD)bcopy
	mv a.out bcopy
	ls -l bcopy
	-rm -f $(DD)*

cache:		$(SRC)/s8/cache.c $(CCDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/cache.c
	chmod 755 a.out
	-chown root a.out
	-mv cache $(DD)cache
	mv a.out cache
	ls -l cache
	-rm -f $(DD)*

check:		$(SRC)/s8/check.c $(CCDEP) $(SYSDEP)
	$(CC) $(CCFLAGS) -I$(SYSINC) $(SRC)/s8/check.c
	chmod 755 a.out
	-chown root a.out
	-mv check $(DD)check
	mv a.out check
	ls -l check
	-rm -f $(DD)*

clri:		$(SRC)/s8/clri.s $(ASDEP)
	$(AS) $(SRC)/s8/clri.s
	strip a.out
	chmod 755 a.out
	-chown root a.out
	-mv clri $(DD)clri
	mv a.out clri
	ls -l clri
	-rm -f $(DD)*

clrm:		$(SRC)/s8/clrm.c $(CCDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/clrm.c
	chmod 755 a.out
	-chown root a.out
	-mv clrm $(DD)clrm
	mv a.out clrm
	ls -l clrm
	-rm -f $(DD)*

config:		$(SRC)/s8/config.c $(CCDEP) $(LSDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/config.c -lS
	chmod 755 a.out
	-chown root a.out
	-mv config $(DD)config
	mv a.out config
	ls -l config
	-rm -f $(DD)*

cron:		$(SRC)/s8/cron.c $(CCDEP) $(CTIME)
	$(CC) $(CCFLAGS) $(SRC)/s8/cron.c
	chmod 755 a.out
	-chown root a.out
	-mv cron $(DD)cron
	mv a.out cron
	ls -l cron
	-rm -f $(DD)*

cu:		$(SRC)/s8/cu.c $(CCDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/cu.c
	chmod 755 a.out
	-chown root a.out
	-mv cu $(DD)cu
	mv a.out cu
	ls -l cu
	-rm -f $(DD)*

dcheck:		$(SRC)/s8/dcheck.c $(CCDEP) $(SYSDEP)
	$(CC) $(CCFLAGS) -I$(SYSINC) $(SRC)/s8/dcheck.c
	chmod 755 a.out
	-chown root a.out
	-mv dcheck $(DD)dcheck
	mv a.out dcheck
	ls -l dcheck
	-rm -f $(DD)*

devnm:		$(SRC)/s8/devnm.c $(CCDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/devnm.c
	chmod 755 a.out
	-chown root a.out
	-mv devnm $(DD)devnm
	mv a.out devnm
	ls -l devnm
	-rm -f $(DD)*

dump:		$(SRC)/s8/dump.c $(CCDEP) $(CTIME) $(SYSDEP)
	$(CC) $(CCFLAGS) -I$(SYSINC) $(SRC)/s8/dump.c
	chmod 755 a.out
	-chown root a.out
	-mv dump $(DD)dump
	mv a.out dump
	ls -l dump
	-rm -f $(DD)*

fcheck:		$(SRC)/s8/fcheck.c $(CCDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/fcheck.c
	chmod 755 a.out
	-chown root a.out
	-mv fcheck $(DD)fcheck
	mv a.out fcheck
	ls -l fcheck
	-rm -f $(DD)*

fsdb:		$(SRC)/s8/fsdb.c $(CCDEP) $(CTIME) $(LSDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/fsdb.c -lS
	chmod 755 a.out
	-chown root a.out
	-mv fsdb $(DD)fsdb
	mv a.out fsdb
	ls -l fsdb
	-rm -f $(DD)*

getty:		$(SRC)/s8/getty.c $(CCDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/getty.c
	chmod 755 a.out
	-chown root a.out
	-mv getty $(DD)getty
	mv a.out getty
	ls -l getty
	-rm -f $(DD)*

glob:		$(SRC)/s8/glob.c $(CCDEP)
	$(CC) $(CCFLAGS) -n $(SRC)/s8/glob.c
	chmod 755 a.out
	-chown root a.out
	-mv glob $(DD)glob
	mv a.out glob
	ls -l glob
	-rm -f $(DD)*

icheck:		$(SRC)/s8/icheck.c $(CCDEP) $(SYSDEP)
	$(CC) $(CCFLAGS) -I$(SYSINC) $(SRC)/s8/icheck.c
	chmod 755 a.out
	-chown root a.out
	-mv icheck $(DD)icheck
	mv a.out icheck
	ls -l icheck
	-rm -f $(DD)*

init:		$(SRC)/s8/init.c $(CCDEP)
	$(CC) $(CCFLAGS) -n $(SRC)/s8/init.c
	chmod 755 a.out
	-chown root a.out
	-mv init $(DD)init
	mv a.out init
	ls -l init
	-rm -f $(DD)*

labelit:	$(SRC)/s8/labelit.c $(CCDEP) $(CTIME) $(LSDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/labelit.c -lS
	chmod 755 a.out
	-chown root a.out
	-mv labelit $(DD)labelit
	mv a.out labelit
	ls -l labelit
	-rm -f $(DD)*

lastcom:		$(SRC)/s8/lastcom.c $(CCDEP) $(CTIME) $(LPWDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/lastcom.c -lPW
	chmod 755 a.out
	-chown root a.out
	-mv lastcom $(DD)lastcom
	mv a.out lastcom
	ls -l lastcom
	-rm -f $(DD)*

log:
	mkdir log
	chmod 755 log
	-chown root log
	ls -ld log

mkfs:		$(SRC)/s8/mkfs.c $(CCDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/mkfs.c
	chmod 755 a.out
	-chown root a.out
	-mv mkfs $(DD)mkfs
	mv a.out mkfs
	ls -l mkfs
	-rm -f $(DD)*

mknod:		$(SRC)/s8/mknod.c $(CCDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/mknod.c
	chmod 755 a.out
	-chown root a.out
	-mv mknod $(DD)mknod
	mv a.out mknod
	ls -l mknod
	-rm -f $(DD)*

mount:		$(SRC)/s8/mount.c $(CCDEP) $(CTIME)
	$(CC) $(CCFLAGS) $(SRC)/s8/mount.c
	chmod 755 a.out
	-chown root a.out
	-mv mount $(DD)mount
	mv a.out mount
	ls -l mount
	-rm -f $(DD)*

ncheck:		$(SRC)/s8/ncheck.c $(CCDEP) $(SYSDEP)
	$(CC) $(CCFLAGS) -I$(SYSINC) $(SRC)/s8/ncheck.c
	chmod 755 a.out
	-chown root a.out
	-mv ncheck $(DD)ncheck
	mv a.out ncheck
	ls -l ncheck
	-rm -f $(DD)*

restor:		$(SRC)/s8/restor.c $(CCDEP) $(CTIME) $(SYSDEP)
	$(CC) $(CCFLAGS) -I$(SYSINC) $(SRC)/s8/restor.c
	chmod 755 a.out
	-chown root a.out
	-mv restor $(DD)restor
	mv a.out restor
	ls -l restor
	-rm -f $(DD)*

rmall:		$(SRC)/s8/rmall.c $(CCDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/rmall.c
	chmod 755 a.out
	-chown root a.out
	-mv rmall $(DD)rmall
	mv a.out rmall
	ls -l rmall
	-rm -f $(DD)*

sa:		$(SRC)/s8/sa.c $(CCDEP) $(CTIME)
	$(CC) $(CCFLAGS) $(SRC)/s8/sa.c
	chmod 755 a.out
	-chown root a.out
	-mv sa $(DD)sa
	mv a.out sa
	ls -l sa
	-rm -f $(DD)*

setmnt:		$(SRC)/s8/setmnt.c $(CCDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/setmnt.c
	chmod 755 a.out
	-chown root a.out
	-mv setmnt $(DD)setmnt
	mv a.out setmnt
	ls -l setmnt
	-rm -f $(DD)*

setuid:		$(SRC)/s8/setuid.c $(CCDEP) $(LPWDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/setuid.c -lPW
	chmod 755 a.out
	-chown root a.out
	-mv setuid $(DD)setuid
	mv a.out setuid
	ls -l setuid
	-rm -f $(DD)*

sysfix:		$(SRC)/s8/sysfix.c $(CCDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/sysfix.c
	chmod 755 a.out
	-chown root a.out
	-mv sysfix $(DD)sysfix
	mv a.out sysfix
	ls -l sysfix
	-rm -f $(DD)*

umount:		$(SRC)/s8/umount.c $(CCDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/umount.c
	chmod 755 a.out
	-chown root a.out
	-mv umount $(DD)umount
	mv a.out umount
	ls -l umount
	-rm -f $(DD)*

unlink:		$(SRC)/s8/unlink.c $(CCDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/unlink.c
	chmod 755 a.out
	-chown root a.out
	-mv unlink $(DD)unlink
	mv a.out unlink
	ls -l unlink
	-rm -f $(DD)*

vpd:		$(SRC)/s8/vpd.c $(CCDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/vpd.c
	chmod 4755 a.out
	-chown root a.out
	-mv vpd $(DD)vpd
	mv a.out vpd
	ls -l vpd
	-rm -f $(DD)*

vc10:		$(SRC)/s8/volcopy.c $(CCDEP) $(CTIME)
	$(CC) -DNBLOCKS=10 $(CCFLAGS) $(SRC)/s8/volcopy.c
	chmod 755 a.out
	-chown root a.out
	-mv vc10 $(DD)vc10
	mv a.out vc10
	ls -l vc10
	-rm -f $(DD)*

vc50:		$(SRC)/s8/volcopy.c $(CCDEP) $(CTIME)
	$(CC) -DNBLOCKS=50 $(CCFLAGS) $(SRC)/s8/volcopy.c
	chmod 755 a.out
	-chown root a.out
	-mv vc50 $(DD)vc50
	mv a.out vc50
	ls -l vc50
	-rm -f $(DD)*

vc88:		$(SRC)/s8/volcopy.c $(CCDEP) $(CTIME)
	$(CC) -DNBLOCKS=88 $(CCFLAGS) $(SRC)/s8/volcopy.c
	chmod 755 a.out
	-chown root a.out
	-mv vc88 $(DD)vc88
	mv a.out vc88
	ls -l vc88
	-rm -f $(DD)*

wall:		$(SRC)/s8/wall.c $(CCDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/wall.c
	chmod 755 a.out
	-chown root a.out
	-mv wall $(DD)wall
	mv a.out wall
	ls -l wall
	-rm -f $(DD)*

wunt:		$(SRC)/s8/wunt.c $(CCDEP) $(LPWDEP)
	$(CC) $(CCFLAGS) $(SRC)/s8/wunt.c -lPW
	chmod 755 a.out
	-chown root a.out
	-mv wunt $(DD)wunt
	mv a.out wunt
	ls -l wunt
	-rm -f $(DD)*

checklist:		# File read by check.
	echo "\n***** File `checklist', for `check', is missing. *****\n"

master:		# File read by config.
	echo "\n***** File `master', for `config', is missing. *****\n"

mnttab:		# File read by mount & umount.
	echo "\n***** File `mnttab', for `mount' & `umount', is missing. *****\n"

mkrp03 mkrp04 rc shutdown startup whodo:
	echo "\n***** Shell `$@' is missing. *****\n"

filesave.x getoff group motd passwd sha ttys utmp:
	echo "\n***** File `$@' is missing. *****\n"

RC:	# dummy name to force re-compiling things.
