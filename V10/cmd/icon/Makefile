SHELL=/bin/sh


what:
		@echo "what do you want to make?"

#
#  Setup system configuration.
#

Setup:
		cd setup/$(name);	make -f ../generic/Makefile Setup

#
# Status information.
#

Status:
		@cat setup/$(name)/status

Status-all:
		make Status name=amdahl_uts
		make Status name=att3b5
		make Status name=att3b20
		make Status name=hp9000
		make Status name=pc_pcix
		make Status name=pc_xenix_smm
		make Status name=pdp11_v7
		make Status name=ridge
		make Status name=sun
		make Status name=unixpc
		make Status name=vax_bsd
		make Status name=vax_sysv

#
# Build a prototype for a new system directory.
#

System:
		mkdir setup/$(name)
		cp setup/common/* setup/$(name)

#
# Compilation and installation.
#

Icon:
		cd src;			make

Install:
		sh Copybin

#
# Construction of auxiliary components.
#

PI:
		cd pi;			make

Ipl:
		cd ipl;			make

#
# Testing.
#

Test-cotest:
		cd tests;		make Test-cotest

Test-icon:
		cd tests;		make Test-icon

Test-extra:
		cd tests;		make Test-extra

Test-expr:
		cd tests;		make Test-expr

Test-check:
		cd tests;		make Test-check

Test-smodel:
		cd tests;		make Test-smodel

Test-lmodel:
		cd tests;		make Test-lmodel

Test-work:
		cd tests;		make Test-work

Test-lgc:
		cd tests;		make Test-lgc

Test-sgc:
		cd tests;		make Test-sgc

Test-lcoexpr:
		cd tests;		make Test-lcoexpr

Test-scoexpr:
		cd tests;		make Test-scoexpr

Test-over:
		cd tests;		make Test-over

Test-pi:
		cd tests;		make Test-pi

Test-ipl:
		cd tests;		make Test-ipl

Test-large:
		cd tests;		make Test-large

Test-small:
		cd tests;		make Test-small

#
# Clean-up.
#

Clean:
		cd src;			make Clean
		cd pi;			make Clean

#  The entries here and in other Makefiles that start with "Dist" are
#  used only at the University of Arizona for preparing the Icon system
#  for distribution. The program gcomp, which is used to specify
#  files not to remove, is from the Icon program library.

Dist-build:
		make Setup-vax_bsd
		make Icon
		make Install
		make PI
		make Ipl

Dist-clean:
		cd src;			make Dist-clean
		cd tests;		make Dist-clean
		cd pi;			make Dist-clean
		cd setup;		make Dist-clean
		cd ipl;			make Dist-clean
		cd docs;		make Dist-clean
		cd book;		make Dist-clean
		rm -f .*K* .emacs*
		rm -f src/iconx/rswitch.[cs]
		rm -f src/iconx/rover.[cs]
		rm -f src/iconx/rlocal.c
		rm -f src/*/fncdef.h
		rm -f src/h/config.h src/h/header.h src/h/paths.h
		rm -rf `gcomp src docs tests README Makefile book ipl work setup pi`
		mkdir bin
		touch bin/.placeholder

Dist-dist:
		rm -rf work
		rm -rf setup/work

Dist-unix:
		mv docs junk
		mv junk/unix docs
		rm -rf junk
		cd setup;		make Dist-unix

Dist-vms:
		rm -rf pi README
		mv docs junk
		mv junk/vms docs
		rm -rf junk
		cd tests;		make Dist-vms
		cd setup;		make Dist-vms
		cd ipl;			make Dist-vms
		rm src/icont/ixhdr.c
		find . -name Makefile -exec rm {} \;

Dist-dos:
		rm -rf pi README
		mv docs junk
		mv junk/dos docs
		rm -rf junk
		cd tests;		make Dist-dos
		cd setup;		make Dist-dos
		cd ipl;			make Dist-dos
		rm src/icont/ixhdr.c
		find . -name Makefile -exec rm {} \;

Dist-smm:
		rm -rf docs
		cd tests;		make Dist-vms
