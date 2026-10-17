SHELL=/bin/sh
what:
		@echo "what do you want to make?"

Test-cotest:
		sh Cotest

Test-icon:
		sh Testd icon

Test-extra:
		sh Test extra

Test-expr:
		sh Test expr

Test-check:
		sh Test check

Test-smodel:
		sh Test smodel

Test-lmodel:
		sh Test lmodel

Test-work:
		sh Testd work

Test-lgc:
		sh Test lgc

Test-sgc:
		sh Test sgc

Test-scoexpr:
		sh Test scoexpr

Test-lcoexpr:
		sh Test lcoexpr

Test-over:
		sh Test over

Test-ipl:
		sh Ipltest

Test-pi:
		sh Pimake
		sh Pitest pi

Test-large:
		make Test-icon
		make Test-extra
		make Test-expr
		make Test-check
		make Test-work
		make Test-lgc
		make Test-lmodel

Test-small:
		make Test-icon
		make Test-extra
		make Test-expr
		make Test-check
		make Test-work
		make Test-sgc
		make Test-smodel

Clean:
		rm -f local/*
		touch local/.placeholder

Dist-clean:
		rm -f .*K* .emacs*
		rm -f local/*
		touch local/.placeholder
		rm -rf `gcomp *test Test* Makefile local stand *.icn *.dat *.ls\
		   fdef.h Pimake`

Dist-vms:
		rm -f Pimake Pitest pi.ls Cotest Ipltest Test Testd
		rm -f fdef.h math.icn trig.icn getenv.icn seek.icn
		rm -f sgc.ls scoexpr.ls smodel.ls smodel.icn
		cd stand; rm -f math.out trig.out getenv.out seek.out
		cd stand; rm -f smodel.out

Dist-smm:
		rm -f lgc.ls lcoexpr.ls lmodel.ls lmodel.icn stand/lmodel.out
