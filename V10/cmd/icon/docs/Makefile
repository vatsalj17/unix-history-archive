SHELL=/bin/sh
troff=psroff

what:
		@echo "what do you want to make?"

Cover:
		$(troff) cover.roff

Distpack:
		$(troff) distpack.roff

Icon_pi:
		$(troff) -man icon_pi.1

Icont:
		$(troff) -man icont.1

Trouble:
		$(troff) trouble.roff

TR83-3:
		$(troff) tr83-3.roff

TR86-10:
		$(troff) tr86-10.roff

TR86-11:
		$(troff) tr86-11.roff

TR86-12:
		$(troff) tr86-12.roff

TR86-13:
		$(troff) tr86-13.roff

Dist-clean:
		rm -f .*K* .emacs*
		rm -rf `gcomp Makefile *.roff *.1 tmac.tr README`
