/*
 * This is the co-expression context switch for the Amdahl 580 operating
 * under UNIX Sys5  5.2.4
 */


/*
 * coswitch
 */
coswitch(old_cs, new_cs, first)
int *old_cs, *new_cs;
int first;
{

   asm("  l	2,68(11)");		/* r0 = old_cs */
   asm("  l	1,64(11)");		/* r1 = new_cs */
   asm("  st 	11,0(2)");		
   asm("  st 	12,4(2)");		
   asm("  st	13,8(2)");		
   asm("  st	8,12(2)");
   asm("  st  	7,16(2)");
   asm("  st    6,20(2)");
   asm("  st	5,24(2)");
   asm("  st    4,28(2)");
   asm("  st    9,32(2)");
   asm("  st 	10,36(2)");
   if (first == 0) { /* this is first activation */
      asm("  l	13,0(1)");
      asm("  sr 12,12");
      asm("  sr 11,11");
      interp(0, 0);
      syserr("interp() returned in coswitch");
      }
   else {
   asm("  l 	11,0(1)");		
   asm("  l 	12,4(1)");	
   asm("  l	13,8(1)");
   asm("  l	8,12(1)");
   asm("  l  	7,16(1)");
   asm("  l     6,20(1)");
   asm("  l	5,24(1)");
   asm("  l     4,28(1)");
   asm("  l     9,32(1)");
   asm("  l 	10,36(1)");
      }
}
