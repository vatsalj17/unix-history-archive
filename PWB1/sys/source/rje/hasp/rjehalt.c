#
/* rjehalt */


/* This information is proprietary and is the property of Bell
   Telephone Laboratories, Incorporated.  Its reproduction or
   disclosure to others, either orally or in writing, is pro-
   hibited without written permission of Bell Laboratories. */


rjehalt() {return ("~|^`rjehalt.c 1.4 4/23/76");};

char who[10];

char stopf[20],deadf[20],soff[20];
char strx[30],phone[30];

main(ac,av)
 char *av[];
 {int sb[18];
  if (alias(av[0],who)<0)
   {printf("%s?\n",av[0]); exit(~0);};
  if (lookup(who,sb,strx,phone)<0 || chdir(sb)<0)
   {printf("Cannot locate %s.\n",who); exit(~0);};
  cat(stopf,who,"stop");
  cat(deadf,who,"dead");
  cat(soff,who,"soff");
  if (((phone[0] >= '0') && (phone[0] <= '9')) || (phone[0] == '-'))
       if ((stat(soff,sb)<0) && (creat(soff,0)<0))
	  {printf("Permission denied.\n"); exit(~0);};
  if (stat(stopf,sb)<0 && creat(stopf,0)<0)
   {printf("Permission denied.\n"); exit(~0);};
  while (stat(deadf,sb)<0) sleep(1);
  *who=+('A'-'a');
  printf("%s halted.\n",who);
  exit(0);};
