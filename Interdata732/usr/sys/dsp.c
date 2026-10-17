/*
 * hex display interrupt routine
 *	- update & panic
 *	 (for dire emergencies only!)
 */


dispint(dev, stat, r0,r1,r2,r3,r4,r5,r6,asp,rf,ps,loc,nps)
{
	printf("Sp %x Psw %x %x\n", asp, ps, loc);
	panic("Display console");
}
