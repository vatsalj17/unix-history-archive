echo	updating $1 $2 $3 $4 $5 $6 $7 $8 $9 in qrymod library
cc	-c -f -O	$1 $2 $3 $4 $5 $6 $7 $8 $9
ar	vu	../../lib/qrymod
rm	*.o
echo	done
