NROFF=	nroff $(RFLAGS)
NMACS=	naa
TROFF=	troff $(RFLAGS)
TMACS=	taa
VROFF=	csh /usr/ucb/vtroff $(RFLAGS)
VMACS=	taa

ref:
	-@$(NROFF) $(NMACS) 0aa.nr 1aa.nr toc.nr
	-@chdir quel; $(NROFF) ../$(NMACS) *.nr
	-@chdir unix; $(NROFF) ../$(NMACS) *.nr
	-@chdir files; $(NROFF) ../$(NMACS) *.nr
	-@chdir error; $(NROFF) ../$(NMACS) *.nr

tref:
	-@$(TROFF) $(TMACS) 0aa.nr 1aa.nr toc.nr
	-@chdir quel; $(TROFF) ../$(TMACS) *.nr
	-@chdir unix; $(TROFF) ../$(TMACS) *.nr
	-@chdir files; $(TROFF) ../$(TMACS) *.nr
	-@chdir error; $(TROFF) ../$(TMACS) *.nr

vref:
	-@$(VROFF) $(VMACS) 0aa.nr 1aa.nr toc.nr
	-@chdir quel; $(VROFF) ../$(VMACS) *.nr
	-@chdir unix; $(VROFF) ../$(VMACS) *.nr
	-@chdir files; $(VROFF) ../$(VMACS) *.nr
	-@chdir error; $(VROFF) ../$(VMACS) *.nr
