#
# This make -ofile should only be run as super-user
#

L = /usr/lib/pascal
B = /usr/bin

DEST = $B/eyacc /usr/lib/libpc.a $L/px_header $B/pc \
	$L/pix $L/how_pc $L/how_pi \
	$L/how_pix $L/how_pxp $L/pc2 \
	$L/pc3 $B/pmerge $B/pxref \
	$L/pc2.0strings $L/pc0  \
	$L/pi2.0strings $L/pi \
	$B/pxp $L/px $B/pascal

SHIP = $B/eyacc /usr/lib/libpc.a $B/pc $B/pmerge $B/pxref \
	$B/pxp $B/pascal $L

DIRS = eyacc libpc pc0 pascal pi px pxp

prepare:
	for i in $(DIRS); do (cd $$i; make -o); done

install:
	for i in $(DIRS); do (cd $$i; echo ==$$i; make -o; make -o install); done
	rm -f $B/pascal
	cp pascal.sh $B/pascal
	chown bin $B/pascal
	chgrp bin $B/pascal
	chmod 775 $B/pascal

clean:
	for i in $(DIRS); do (cd $$i; make -o clean); done

clobber: clean
	rm -f $(DEST)

ship:
	ship $(SHIP)
