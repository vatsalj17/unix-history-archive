NROFF=	nroff
TROFF=	ditroff
PRINTER= Pdp
NMACS=	../naa
TMACS=	../taa
FILES= 0aa.nr 1aa.nr toc.nr 2aa.nr quel/*.nr unix/*.nr files/*.nr error/*.nr

.SILENT

ref:	$(FILES) naa
	$(NROFF) naa 0aa.nr 1aa.nr toc.nr 2aa.nr
	chdir quel; $(NROFF) $(NMACS) *.nr
	chdir unix; $(NROFF) $(NMACS) *.nr
	chdir files; $(NROFF) $(NMACS) *.nr
	chdir error; $(NROFF) $(NMACS) *.nr

tref:	$(FILES) taa
	$(TROFF) -$(PRINTER) -t taa $(FILES) > paper.$(PRINTER)
