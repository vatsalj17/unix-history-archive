echoall
cc -c -O rkdfc.c
as -o rkdfs.o rkdfs.s
ld -s -o unix.rkdf rkdfs.o rkdfc.o -lc -l
rm -f *.o
chmod 0 unix.rkdf
if $1x = x exit
cp unix.rkdf /unix.rkdf
rm -f unix.rkdf
