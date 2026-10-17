echoall
as rk60boot.s
strip a.out
size a.out
dd if=a.out of=rk60boot bs=16 skip=1
rm a.out
ls -l rk60boot
