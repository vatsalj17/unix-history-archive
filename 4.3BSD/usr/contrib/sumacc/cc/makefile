# sun command makefile
# Bill Nowicki December 1981
# merged a bunch of uncommented files
# March 15, 1982 (win) changed /usr/bin to /usr/sun/bin

DESTDIR= /usr/sun/bin

all : cc68 compiler assembler loader install

cc68: cc68.c
	cc -o cc68 cc68.c
compiler:
	cd ccom; make

assembler:
	cd as; make

loader:
	cd ld; make


install: install-cc68 \
 install-compiler install-assembler install-loader $(DESTDIR)/lorder68

install-cc68:
	cp cc68 $(DESTDIR)/cc68

install-compiler:
	cd c2; make install
	cd ccom; make install

install-assembler:
	cd as; make install

install-loader:
	cd ld; make install

clean:
	rm -f dll cc68 dlx *.b *.o *BAK *CKP
	-cd as; make clean
	-cd c2; make clean
	-cd ccom; make clean
	-cd ddt68; make clean
	-cd ld; make clean
