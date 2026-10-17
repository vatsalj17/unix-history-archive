#
sptotab (pcurcol, firstab, tabinc)
int     pcurcol,
        firstab,
        tabinc;
{
    register short
                    curcol,
                    diff;

    curcol = pcurcol;
    firstab++;

    if (curcol <= firstab)
	return (firstab - curcol);
    else
	return ((diff = (--curcol % tabinc)) == 0
		? 0
		: (tabinc - diff));
}
