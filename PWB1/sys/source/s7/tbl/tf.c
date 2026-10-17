 /* tf.c: save and restore fill mode around table */
# include "t..c"
savefill()
{
/* remembers the status of fill mode in nr 45 (SF) */
/* nr SF = 0 means no fill, 1 means fill */
fprintf(tabout, ".nr %d \\n(.u\n", SF);
fprintf(tabout, ".nf\n");
}
rstofill()
{
fprintf(tabout, ".if \\n(%d>0 .fi\n", SF);
}
endoff()
{
int i;
	for(i=0; i<MAXHEAD; i++)
		if (linestop[i])
			fprintf(tabout, ".nr #%c 0\n", 'a'+i);
	for(i=0; i<texct; i++)
		fprintf(tabout, ".rm %c+\n",texstr[i]);
fprintf(tabout, "%s\n", last);
}
ifdivert()
{
fprintf(tabout, ".ds #d .d\n");
fprintf(tabout, ".if \\(ts\\n(.z\\(ts\\(ts .ds #d nl\n");
}
saveline()
{
fprintf(tabout, ".nr %d \\n(c.\n",SL);
linstart=iline;
}
restline()
{
fprintf(tabout, ".nr c. \\n(%d+%d\n",SL, iline-linstart);
}
cleanfc()
{
fprintf(tabout, ".fc\n");
}
