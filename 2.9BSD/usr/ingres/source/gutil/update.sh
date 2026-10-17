echo	updating $1 $2 $3 $4 $5 $6 $7 $8 $9 in gutil libraries
cc	-c -f -O	$1 $2 $3 $4 $5 $6 $7 $8 $9
ar	vu	../../lib/gutil
ar	vu	/lib/libgu.a
rm	*.o
echo	done
