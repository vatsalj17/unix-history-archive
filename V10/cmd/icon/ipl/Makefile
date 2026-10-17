SHELL=/bin/sh

All:
		make Progs
		make Procs

Progs:
		sh Compile

Procs:
		sh Translate

Dist-clean:
		cd source;		make Dist-clean
		rm -f *.K* .emacs*
		rm -f progs/* procs/*
		touch progs/.placeholder procs/.placeholder
		rm -rf `gcomp Makefile Compile Translate data source procs\
		   progs *.ls`

Dist-vms:
		rm -f Compile Translate
