trycomp(){
	rm -f obj
	./a.out /usr/ark/interp.p
	cmp -l obj /usr/ark/obj
}

recomp(){
	rm $1.o
	make -o
}

try(){
	recomp $1
	trycomp
}
