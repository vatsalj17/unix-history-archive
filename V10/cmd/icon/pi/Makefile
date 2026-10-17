SHELL=/bin/sh
all:
		rm -f rtlib linklib itran
		rm -rf h icont iconx link
		mkdir h icont iconx link
		cp ../src/tran/itran .
		strip itran
		cp ../src/h/*.h h
		cp ../src/iconx/*.h iconx
		cp ../src/link/ilink.h link
		cp ../src/link/builtin.c ../src/link/ilink.c link
		cp ../src/iconx/rconv.c ../src/iconx/idata.c iconx
		cp ../src/icont/icont.c ../src/icont/ixhdr.c icont
		sh Pilib

Clean:
		rm -f iconx/*.o

Dist-clean:
		cd pil;			make Dist-clean
		rm -f .BK* .CK* .emacs*
		rm -rf `gcomp Makefile Pilib pil`
