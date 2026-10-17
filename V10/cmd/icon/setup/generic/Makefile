all:
		@echo "what do you want to make"

Setup:
		make -f ../generic/Makefile Headers
		make -f ../generic/Makefile Makefiles
		make -f ../generic/Makefile Scripts
		make -f ../generic/Makefile Localcode
		make -f ../generic/Makefile PI

Headers:
		cp paths.h ../../src/h
		cat header.hdr ../generic/header.trl >../../src/h/header.h
		cat ../generic/config.hdr config.trl ../generic/config.typ \
		   >../../src/h/config.h

Makefiles:
		cat icont.hdr ../generic/icont.make > ../../src/icont/Makefile
		cat iconx.hdr ../generic/iconx.make > ../../src/iconx/Makefile
		cat link.hdr ../generic/link.make > ../../src/link/Makefile
		cat tran.hdr ../generic/tran.make > ../../src/tran/Makefile
		cat pi.hdr ../generic/pi.make > ../../Pimakefile

Scripts:
		$(CC) -E setpath.c | egrep -v '(^#|^$$)' > ../../Setpath
		$(CC) -E copybin.c | egrep -v '(^#|^$$)' > ../../Copybin
		ed - ../../Copybin <../generic/dequote.ed

Localcode:
		rm -f ../../src/iconx/rswitch.[cs]
		rm -f ../../src/iconx/rover.[cs]
		rm -f ../../src/iconx/rlocal.c
		rm -f ../../src/*/fncdef.h
		cp rswitch.[cs] ../../src/iconx
		cp rover.[cs] ../../src/iconx
		cp rlocal.c ../../src/iconx

PI:
		$(CC) -E icon_pi.c | egrep -v '(^#|^$$)' > ../../icon_pi
		ed - ../../icon_pi <../generic/dequote.ed
		chmod 755 ../../icon_pi
		cp Ranlib ../../Ranlib
		chmod 755 ../../Ranlib
