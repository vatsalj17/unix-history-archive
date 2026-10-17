.SUFFIXES: .rsrc .b .ln .s .c
BIN=
INCLUDE=/usr/include/mac

CFILES = desk.c cal.c
HFILES = device.h
SFILES = crtcal.s

.c.b:
	$(BIN)cc68 -I$(INCLUDE) -c $<

.c.s:
	$(BIN)cc68 -I$(INCLUDE) -S $<

.s.b:
	$(BIN)cc68 -c $<

.c.ln:
	$(BIN)lint -I$(INCLUDE) -lmac $< > $*.ln

all: desk.rsrc

cal: cal.b crtcal.b 
	$(BIN)ld68 -X -r -d -e drvr -T 0 crtcal.b cal.b -lmac -lc -x -o cal

desk: desk.b
	$(BIN)cc68 -m desk.b -o desk

desk.rsrc: cal desk desk.rc
	$(BIN)rmaker desk.rc

put: desk.rsrc
	$(BIN)macput -r desk

lint: cal.ln desk.ln

desk.shar: $(CFILES) $(SFILES) $(HFILES)
	csh shar desk.shar $(CFILES) $(SFILES) $(HFILES) desk.rc Makefile

clean:
	rm -f *.b *.rsrc cal desk *- z

