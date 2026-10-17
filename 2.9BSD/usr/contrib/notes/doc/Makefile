#	Makefile for the notesfile reference manual
#	March 29, 1982
#	Ray Essick, University of Illinois
#	uiucdcs!essick     uiucdcs!notes
#
#       Modified by Rick Spickelmier, UCB
#       Modified by Lou Salkind, NYU

NROFF = nroff
FILES = 0 1 2 3 4 5 a.checklist b.future c.update

doc:	$(FILES) macros vtoc
	$(NROFF) macros $(FILES) vtoc > doc

pretty: summary
	pr -t -2 -w80 -l45 summary >pretty
