char	un[8];
main()
{
	register i;
	uname(un);
	for(i=0;i<8;i++)
		if(un[i])
			write(1,&un[i],1); else
			break;
	write(1,"\n",1);
}
