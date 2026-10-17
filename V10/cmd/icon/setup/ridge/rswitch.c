/*
 * This is the co-expression context switch for the Ridge 32 operating
 * under ROS 3.
 */


/*
 * coswitch
 */
coswitch(old_cs, new_cs, first)
int *old_cs, *new_cs;
int first;
{
   asm("  load r0,r15,24");		/* r0 = old_cs */
   asm("  load r1,r15,32");		/* r1 = new_cs */
   asm("  store r14,r0,0");		/* save sp in cstate[0] */
   asm("  store r15,r0,4");		/* save r5 (fp) in cstate[1] */
   asm("  store r6,r0,8");		/* save r6-r13 */
   asm("  store r7,r0,12");
   asm("  store r8,r0,16");
   asm("  store r9,r0,20");
   asm("  store r10,r0,24");
   asm("  store r11,r0,28");
   asm("  store r12,r0,32");
   asm("  store r13,r0,36");
   if (first == 0) { /* this is first activation */
      asm("  load r14,r1,0");
      asm("  laddr r14,r14,-40");
      asm("  sub r15,r15");
      interp(0, 0);
      syserr("interp() returned in coswitch");
      }
   else {
      asm("  load r14,r1,0");
      asm("  load r15,r1,4");
      asm("  load r6,r1,8");
      asm("  load r7,r1,12");
      asm("  load r8,r1,16");
      asm("  load r9,r1,20");
      asm("  load r10,r1,24");
      asm("  load r11,r1,28");
      asm("  load r12,r1,32");
      asm("  load r13,r1,36");
      }
}
