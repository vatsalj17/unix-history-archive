CC = cc
AS = as
LD = ld
LDFLAGS = -s

iltd: iltd.o tcrt0.o console.o hthp.o tmrp.o \
		rp03boot.o rp04boot.o
	$(LD) $(LDFLAGS) -o iltd tcrt0.o iltd.o console.o hthp.o tmrp.o \
		rp03boot.o rp04boot.o -lc -l

rp03boot.o:	../rp03boot.s
	$(AS) ../rp03boot.s
	dd if=a.out of=a.out1 bs=16 skip=1 count=32
	-od a.out1 >a.out
	-sed -e "s/^........//" -e "s/ /; /g" a.out >a.out1
	-(echo ".globl _rp03boot;_rp03boot:";cat a.out1;echo "0;0;0;0;0;0;0;0")>tmp.s
	$(AS) tmp.s
	-mv a.out rp03boot.o
	rm tmp.s a.out1

rp04boot.o:	../rp04boot.s
	$(AS) ../rp04boot.s
	dd if=a.out of=a.out1 bs=16 skip=1 count=32
	-od a.out1 >a.out
	-sed -e "s/^........//" -e "s/ /; /g" a.out>a.out1
	-(echo ".globl _rp04boot;_rp04boot:";cat a.out1;echo "0;0;0;0;0;0;0;0")>tmp.s
	$(AS) tmp.s
	-mv a.out rp04boot.o
	rm tmp.s a.out1

clean:
	-rm -f iltd.o tcrt0.o console.o hthp.o tmrp.o \
		rp03boot.o rp04boot.o >/dev/null
