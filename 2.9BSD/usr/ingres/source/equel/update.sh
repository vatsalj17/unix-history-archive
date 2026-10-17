echo	updating $1 $2 $3 $4 $5 $6 $7 $8 $9 in .../lib/equel
cc	-c -O $1 $2 $3 $4 $5 $6 $7 $8 $9
ar	vu	../../lib/equel
rm	*.o
echo	.../lib/equel updated
echo	compiling equel
cc	-f -O	../../lib/equel	-ly	-lgu
mv	a.out	equel
chmod	711	equel
echo	"new equel in .../source/equel/equel"
echo	"move it to .../bin/equel and /usr/bin/equel"
