OBJS = hangman.b clrtoeol.b endgame.b extern.b getguess.b getword.b playgame.b prdata.b prman.b prword.b setup.b

.SUFFIXES: .rsrc .b

.c.b:
	cc68 -c -O $<

.b.rsrc:
	cc68 -z -m -O *.b
	rmaker $*.rc

hangman.rsrc: $(OBJS) hangman.rc

clean:
	rm -f *.b *.rsrc
