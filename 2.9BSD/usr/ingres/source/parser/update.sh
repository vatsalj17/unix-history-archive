echo	updating $1 $2 $3 $4 $5 $6 $7 $8 $9 in parser library
cc	-c -O	$1 $2 $3 $4 $5 $6 $7 $8 $9
ar	vu	../../lib/parser
rm	[a-x]*.o
echo	done
