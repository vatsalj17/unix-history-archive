/*
 * coswitch
 */
coswitch(old_cs, new_cs, first)
int *old_cs, *new_cs;
int first;
{
   asm("  movw 0(%ap),%r0");
   asm("  movw 4(%ap),%r1");
   asm("  movw %sp,0(%r0)");
   asm("  movw %fp,4(%r0)");
   asm("  movw %ap,8(%r0)");
   asm("  movw %r3,12(%r0)");
   asm("  movw %r4,16(%r0)");
   asm("  movw %r5,20(%r0)");
   asm("  movw %r6,24(%r0)");
   asm("  movw %r7,28(%r0)");
   asm("  movw %r8,32(%r0)");
   if (first == 0) { /* this is first activation */
      asm("  movw 0(%r1),%sp");
      asm("  movw &0,%fp");
      asm("  movw &0,%ap");
      interp(0, 0);
      syserr("interp() returned in coswitch");
      }
   else {
      asm("  movw 0(%r1),%sp");
      asm("  movw 4(%r1),%fp");
      asm("  movw 8(%r1),%ap");
      asm("  movw 12(%r1),%r3");
      asm("  movw 16(%r1),%r4");
      asm("  movw 20(%r1),%r5");
      asm("  movw 24(%r1),%r6");
      asm("  movw 28(%r1),%r7");
      asm("  movw 32(%r1),%r8");
      }
}
