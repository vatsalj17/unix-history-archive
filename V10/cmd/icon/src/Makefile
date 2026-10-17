SHELL=/bin/sh
all:
		cd icont;	make
		cd iconx;	make
		cd link;	make
		cd tran;	make

Clean:
		cd icont; rm -f *.o iconx.hdr icont
		cd iconx; rm -f *.o iconx
		cd link; rm -f *.o ilink
		cd tran; rm -f *.o itran

Dist-clean:
		cd h; (rm -f .*K* .emacs* config.h; rm -rf `gcomp *.[h]`)
		cd icont; (rm -f .*K* .emacs*; rm -rf `gcomp *.[ch]`)
		cd iconx; (rm -f .*K* .emacs* rlocal.*; rm -rf `gcomp *.[ch]`)
		cd link; (rm -f .*K* .emacs*; rm -rf `gcomp *.[ch]`)
		cd tran; (rm -f .*K* .emacs*;\
		   rm -rf `gcomp *.c *.h mkkeytab.icn mktoktab.icn \
		   tokens optab keywords icon.g pscript`)
		rm -f .*K* .emacs*
		rm -rf `gcomp Makefile h icont iconx link tran`
