echo	updating $1 $2 $3 $4 $5 $6 $7 $8 $9 in iutil libraries
cc	-c -f -O	$1 $2 $3 $4 $5 $6 $7 $8 $9
ar	vu	../../lib/iutil
ar	vu	/lib/libiu.a
rm	*.o
echo	done
