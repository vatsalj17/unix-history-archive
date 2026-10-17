
OBJS=		char.o code.o err.o itran.o keyword.o lex.o lnklist.o mem.o \
		   optab.o parse.o sym.o toktab.o

itran:		$(OBJS)
		$(CC) $(LDFLAGS) -o itran $(OBJS)

$(OBJS):	../h/config.h

char.o:		char.h
code.o:		itran.h sym.h token.h tree.h tstats.h ../h/config.h
err.o:		itran.h lex.h token.h tree.h
itran.o:	itran.h sym.h token.h tree.h tstats.h ../h/config.h ../h/version.h
keyword.o:	sym.h ../h/keyword.h
lex.o:		char.h itran.h lex.h token.h tree.h
lnklist.o:	itran.h lfile.h
mem.o:		itran.h sym.h tree.h ../h/memsize.h
optab.o:	lex.h
parse.o:	itran.h sym.h tree.h ../h/config.h
sym.o:		char.h itran.h lfile.h sym.h token.h
toktab.o:	itran.h lex.h token.h

#
#  The following sections are commented out because they do not need
#  to be performed unless changes are made to icon.g, keywords, tokens,
#  or optab.  Such changes involve modifications to the syntax of Icon
#  and are not part of the installation process.  However, if the
#  distribution files are unloaded in a fashion such that their dates
#  are not set properly, the following sections would be attempted.
#
#  Note that if any changes are made to the four files mentioned above,
#  the comment characters at the beginning of the following lines should be
#  removed.
#
#parse.c token.h:	icon.g
#			yacc -v -d icon.g	# expect 169 shift/reduce conflicts
#			mv y.tab.c parse.c
#			ed parse.c <pscript
#			mv y.tab.h token.h
#
#keyword.h keyword.c:	keywords
#			mkkeytab
#
#toktab.c optab.c:	tokens optab
#			mktoktab
#
#mkkeytab:		mkkeytab.icn
#			icont -o mkkeytab mkkeytab.icn
#
#mktoktab:		mktoktab.icn
#			icont -o mktoktab mktoktab.icn
