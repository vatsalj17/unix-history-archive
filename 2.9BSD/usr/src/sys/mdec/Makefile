all:	hkuboot hpuboot rkuboot rluboot rmuboot rpuboot

hkuboot: hkuboot.s
	as -o hkuboot.o hkuboot.s
	size hkuboot.o
	strip hkuboot.o
	dd if=hkuboot.o of=hkuboot bs=8w skip=1
	rm hkuboot.o

hpuboot: hpuboot.s
	as -o hpuboot.o hpuboot.s
	size hpuboot.o
	strip hpuboot.o
	dd if=hpuboot.o of=hpuboot bs=8w skip=1
	rm hpuboot.o

rkuboot: rkuboot.s
	as -o rkuboot rkuboot.s
	strip rkuboot

rluboot: rluboot.s
	as -o rluboot.o rluboot.s
	size rluboot.o
	strip rluboot.o
	dd if=rluboot.o of=rluboot bs=8w skip=1
	rm rluboot.o

rmuboot: rmuboot.s
	as -o rmuboot.o rmuboot.s
	size rmuboot.o
	strip rmuboot.o
	dd if=rmuboot.o of=rmuboot bs=8w skip=1
	rm rmuboot.o

rpuboot: rpuboot.s
	as -o rpuboot.o rpuboot.s
	size rpuboot.o
	strip rpuboot.o
	dd if=rpuboot.o of=rpuboot bs=8w skip=1
	rm rpuboot.o

.DEFAULT:
	as -o $@.o $@.s
	size $@.o
	strip $@.o
	dd if=$@.o of=$@ bs=8w skip=1
	rm $@.o
