.SUFFIXES: .rsrc .b .ln .s .c
BIN=
INCLUDE=/usr/include/mac

CFILES = desk.c print.c
SFILES = crtprint.s

.c.b:
	$(BIN)cc68 -I$(INCLUDE) -c $<

.c.s:
	$(BIN)cc68 -I$(INCLUDE) -S $<

.s.b:
	$(BIN)cc68 -c $<

.c.ln:
	$(BIN)lint -I$(INCLUDE) -lmac $< > $*.ln

all: desk.rsrc

print: print.b crtprint.b 
	$(BIN)ld68 -X -r -d -e drvr -T 0 crtprint.b print.b -lmac -lc -x -o print

desk: desk.b
	$(BIN)cc68 -m desk.b -o desk

desk.rsrc: print desk desk.rc
	$(BIN)rmaker desk.rc

put: desk.rsrc
	$(BIN)macput -o -r desk

lint: print.ln desk.ln

desk.shar: $(CFILES) $(SFILES) desk.rc Makefile
	shar desk.shar $(CFILES) $(SFILES) desk.rc Makefile >x.shar

clean:
	rm -f *.b *.rsrc print desk

