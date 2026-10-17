FP=					# -f for machines without floating point
CFLAGS=		-O $(INCLUDES) $(FP)
INCLUDES=
OBJECTS=	account.o	\
		ascii.o		\
		at.o		\
		classify.o	\
		cmdini.o	\
		cmdload.o	\
		cmdrun.o	\
		curdir.o	\
		curses.o	\
		dir.o		\
		dircmd.o	\
		dirlist.o	\
		enterf.o	\
		file.o		\
		grep.o		\
		help.o		\
		main.o		\
		make.o		\
		misccmd.o	\
		options.o	\
		page.o		\
		process.o	\
		readarg.o	\
		remove.o	\
		show.o		\
		showopen.o	\
		strings.o	\
		tty.o		\
		xeq.o
 
all:		vsh

cp:
		./vshrc.gen
		cp vsh ${DESTDIR}/usr/ucb/vsh
		cp genhelp ${DESTDIR}/usr/lib/vsh
		cp rmhelp ${DESTDIR}/usr/lib/vsh
		cp showhelp ${DESTDIR}/usr/lib/vsh
		cp dflt.vshrc ${DESTDIR}/usr/lib/vsh

vsh:   		$(OBJECTS)
		cc -o vsh -n $(FP) -s $(OBJECTS) -ltermcap;  size vsh;

lint:
		lint -bxac *.c

clean:
		rm -f ${OBJECTS} vsh
