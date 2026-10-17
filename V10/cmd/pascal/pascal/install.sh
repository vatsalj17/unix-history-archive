bin=$1
lib=$2

ch() {
	chown bin,bin $1
	chmod o-w,g+w $1
}

copy() {
	rm -f $2/$1
	cp $1 $2
	ch $2/$1
}

scopy() {
	strip $1
	copy $1 $2
}

dd if=px_header of=$lib/px_header conv=sync
scopy pc $bin
scopy pix $lib
copy how_pc $lib
copy how_pi $lib
copy how_pix $lib
copy how_pxp $lib
scopy pc2 $lib
scopy pc3 $lib
scopy pmerge $bin
pc -w pxref.p -o $bin/pxref
ch $bin/pxref
