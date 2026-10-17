
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
#  Create /usr/mdec.
# ------------
MAIN:		FIRST copy iltd list mboot tapeboot tboot \
		rp03boot rp04boot
	-rm -f $(DD)*
	-@echo "\n***** Following Files Still Present:";  ls -l $(DD)*;  echo "\n"
	-@echo "\n***** Following Sticky Files Still Present, Remove After Next Reboot:"; \
			ls -l $(STICKY)*;  echo "\n"

FIRST:
	echo "\n*** You should link uboot to rp03boot or rp04boot. ***\n"

copy:		$(SRC)/util/copy.s $(ASDEP)
	$(AS) $(SRC)/util/copy.s
	strip a.out
	chmod 755 a.out
	-chown bin a.out
	-mv copy $(DD)copy
	mv a.out copy
	ls -l copy
	-rm -f $(DD)*

iltd:		$(SRC)/util/iload/*.[cs] $(SRC)/util/iload/makefile \
		$(CCDEP) $(ASDEP)
	chdir $(SRC)/util/iload; $(MAKE) iltd "CC=$(CC)" "AS=$(AS)"
	mv $(SRC)/util/iload/iltd a.out
	chdir $(SRC)/util/iload; $(MAKE) clean
	chmod 755 a.out
	-chown bin a.out
	-mv iltd $(DD)iltd
	mv a.out iltd
	ls -l iltd
	-rm -f $(DD)*

list:		$(SRC)/util/list.s $(ASDEP)
	$(AS) $(SRC)/util/list.s
	strip a.out
	chmod 755 a.out
	-chown bin a.out
	-mv list $(DD)list
	mv a.out list
	ls -l list
	-rm -f $(DD)*

mboot:		$(SRC)/util/mboot.s $(ASDEP)
	$(AS) $(SRC)/util/mboot.s
	strip a.out
	chmod 755 a.out
	-chown bin a.out
	-mv mboot $(DD)mboot
	mv a.out mboot
	ls -l mboot
	-rm -f $(DD)*

rp03boot:		$(SRC)/util/rp03boot.s $(ASDEP)
	$(AS) $(SRC)/util/rp03boot.s
	strip a.out
	mv a.out a.out1
	dd if=a.out1 bs=16 count=32 skip=1 of=a.out conv=sync
	rm -f a.out1
	chmod 755 a.out
	-chown bin a.out
	-mv rp03boot $(DD)rp03boot
	mv a.out rp03boot
	ls -l rp03boot
	-rm -f $(DD)*

rp04boot:		$(SRC)/util/rp04boot.s $(ASDEP)
	$(AS) $(SRC)/util/rp04boot.s
	strip a.out
	mv a.out a.out1
	dd if=a.out1 bs=16 count=32 skip=1 of=a.out conv=sync
	rm -f a.out1
	chmod 755 a.out
	-chown bin a.out
	-mv rp04boot $(DD)rp04boot
	mv a.out rp04boot
	ls -l rp04boot
	-rm -f $(DD)*

tapeboot:		$(SRC)/util/tapeboot.s $(ASDEP)
	$(AS) $(SRC)/util/tapeboot.s
	strip a.out
	mv a.out a.out1
	dd if=a.out1 of=a.out count=1 bs=512 conv=sync
	cat a.out a.out >a.out1
	rm -f a.out
	chmod 755 a.out1
	-chown bin a.out1
	-mv tapeboot $(DD)tapeboot
	mv a.out1 tapeboot
	ls -l tapeboot
	-rm -f $(DD)*

tboot:		$(SRC)/util/tboot.s $(ASDEP)
	$(AS) $(SRC)/util/tboot.s
	strip a.out
	chmod 755 a.out
	-chown bin a.out
	-mv tboot $(DD)tboot
	mv a.out tboot
	ls -l tboot
	-rm -f $(DD)*


RC:	# dummy name to force re-compiling things.
