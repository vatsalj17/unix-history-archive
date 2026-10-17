extern int 	brkptr;
nountil(p1) int p1; {
	outnum(p1+1);
	outgoto(p1);
	outcont(p1+2);
	brkptr--;
}
