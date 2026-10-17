 /* t9.c: write lines for tables over 200 lines */
# include "t..c"
yetmore()
{
int icol, ch, useln;
for(useln=0; useln<MAXLIN && table[useln]==0; useln++);
if (useln>=MAXLIN)
	error("Wierd.  No data in table.",0);
table[0]=table[useln];
for(useln=nlin-1; useln>=0 && (fullbot[useln] || instead[useln]); useln--);
if (useln<0)
	error("Wierd.  No real lines in table.", 0);
while (gets1(cstore=cspace))
	{
	if (prefix(".TE", cstore))
		break;
	if (cstore[0] == '.' && letter(cstore[1]))
		{
		puts(cstore);
		continue;
		}
	instead[0]=fullbot[0]=0;
	if (cstore[1]==0)
	switch(cstore[0])
		{
		case '_': fullbot[0]= '-'; putline(useln,0);  continue;
		case '=': fullbot[0]= '='; putline(useln, 0); continue;
		}
	for (icol = 0; icol <ncol; icol++)
		{
		table[0][icol].col = cstore;
		table[0][icol].rcol=0;
		for(; (ch= *cstore) != '\0' && ch != tab; cstore++)
				;
		*cstore++ = '\0';
		switch(ctype(useln,icol))
			{
			case 'n':
				table[0][icol].rcol = maknew(table[0][icol].col);
				break;
			case 'a':
				table[0][icol].rcol = table[0][icol].col;
				table[0][icol].col= "";
				break;
			}
		while (ctype(useln,icol+1)== 's') /* spanning */
			table[0][++icol].col = "";
		if (ch == '\0') break;
		}
	while (++icol <ncol)
		table[0][icol].col = "";
	putline(useln,0);
	}
last =cstore;
return;
}
