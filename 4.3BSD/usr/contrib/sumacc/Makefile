# SUMacC makefile

RT=/usr/sun
DP=/usr/sun/bin
DL=/usr/sun/lib
DI=/usr/include

# for Eunice, comment out the lines above and enable the lines below
#MACROOT=/usr/local/mac
#DP=$(MACROOT)/bin
#DL=$(MACROOT)/lib
#DI=$(MACROOT)/include

all:
	-mkdir $(RT)
	-mkdir $(DP) $(DL)
	-mkdir $(DI) $(DI)/mac
	cp  h/*.h $(DI)/mac
	ln -s $(DI)/mac $(RT)/include
	cd cc42; make install
	cd cc; make all
	cp man/*.1 /usr/man/man1
	cd cmd; make DP=$(DP) DL=$(DL) 
	cd cmd; make DP=$(DP) DL=$(DL) install
	cd lib; make DP=$(DP) DL=$(DL) 
	cd lib; make DP=$(DP) DL=$(DL) install
	cd libc; make DP=$(DP) DL=$(DL) install
	cd mac; make DP=$(DP) DL=$(DL) 

clean:
	cd cc42; make clean
	cd cc; make clean
	cd cmd; make clean
	cd lib; make clean
	cd libc; make clean
	cd mac; make clean

