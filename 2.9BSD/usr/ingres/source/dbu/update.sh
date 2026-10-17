echo updating $1 $2 $3 $4 $5 $6 $7 $8 $9 in dbulib
cc -c -O -f $1 $2 $3 $4 $5 $6 $7 $8 $9
ar vu ../../lib/dbulib
rm *.o
echo done
