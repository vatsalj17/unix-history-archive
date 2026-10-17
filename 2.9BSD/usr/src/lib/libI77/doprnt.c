/*
char id_doprnt[] = "@(#)doprnt.c	1.1";
 *
 * doprnt:  Common code for fortran-callable formatted output routines
 * printn, fprntn, sprntn.
 *
 * Adapted by Bruce R. Julian, USGS, March 1980,
 * from function printn, by James W. Herriot, USGS, Feb 1980.
 * Additions (by JWH) to printf format syntax are:
 *    1. %n(          where "n" is number of iterations to loop
 *    2. %na          where "n" is size of array
 *    3. %n{          shorthand for "%na %(" -- "%(" will use previous n
 *    4. %) -or- %}   end of loop
 * note that "n" above may be a constant of a "^" meaning a parameter.
 *
 * Modified by Bruce R. Julian,  USGS, Mar 1980 to:
 *     - handle double precision arrays
 *     - accept all printf formats
 */
#define MAX  200
#include <stdio.h>
#include <ctype.h>
static FILE *File;
static int  Parptr,Subi,Subz,Arr,**Stk;

#ifdef D
static int BUG=0;
#endif

static char Buf[MAX],*Format;
static union {
	char *S; 
	char *C; 
	long *L; 
	double *D;
	int *I;
} 
P;

doprnt(format,params,farg)
FILE *farg;
char format[]; 
long *params[]; 
{
	File = farg;
	Parptr=Arr=0; 
	Stk= params; 
	Format=format;
	recur(0); 
	fflush(File);
}
recur(ptr)
int ptr; 
{
	int i,n,lev,o; 
	char c; 

#ifdef D
	if(BUG)fprintf(File,"recur: %s\n",Format+ptr);
#endif
	while( (o=eatstr(&ptr,&c,&n)) != -1){
#ifdef D
		if(BUG)fprintf(File,"o=%d ptr=%d Buf=[%s] c=%c n=%d\n",o,ptr,Buf,c,n);
#endif
		if(o) {
			for(i=0;i<n;i++)recur(ptr);
			lev=1; 
			while(lev+=eatstr(&ptr,&c,&n)); 
		}
		else{
			switch(c){
			case 's':		/* STRING */
				onepar(1);
				fprintf(File,Buf, P.S); 
				break;
			case 'c':		/* CHARACTER */
				onepar(1);
				fprintf(File,Buf,*P.C); 
				break;
			case 'd':		/* INTEGER*2 */
			case 'o': 
			case 'x': 
				onepar(1);
				fprintf(File,Buf,*P.I); 
				break;
			case 'l':		/* INTEGER *4 */
				onepar(2);
				fprintf(File,Buf,*P.L); 
				break;
			case 'e':		/* REAL */
			case 'f': 
			case 'g': 
				onepar(2);
				fprintf(File,Buf,*P.D); 
				break;
			case 'L':		/* DOUBLE PRECISION */
				onepar(4);
				fprintf(File, Buf, *P.D);
				break;
			default:
				fprintf(File,Buf     ); 
				break;
			}
#ifdef D
			if(BUG)fprintf(File," <--output\n"); 
#endif
		}
	}     
}
#define Next  (*cc=c=Buf[b++]=Format[(*ptr)++])
eatstr(ptr,cc,n)
int *n; 
register int *ptr;
char *cc; 
{
	register int b=0; 
	int rtn=0;
	char c; 

	*n=0;
#ifdef D
	if(BUG)fprintf(File,"eatstr: ptr=%d\n",*ptr);
#endif
	switch(Next){
	case '\0': 
		(*ptr)--; 
		rtn= -1; 
		break;
	case '%': 
		while(Next=='-'||c=='.'||c>='0'&&c<='9')*n= *n*10+c-'0';
		if(c=='^'){
			onepar(0); 
			*n= *P.L; 
			Next;
		}
		switch(c){
		case '\0': 
			(*ptr)--;
		case  '}':
		case  ')': 
			rtn= -1;                             
			break;
		case  '(': 
			*n= (!*n && Arr) ? Subz : *n; 
			rtn=1; 
			break;
		case  '{': 
			rtn=1;
		case  'a': 
			Subz= *n; 
			Arr=1; 
			Subi=b=0;  
			*cc='%'; 
			break;
		case  'n': 
			c='D';
		case 'D':
		case 'O':
		case 'X':
			*cc=Buf[b-1]='l';
			Buf[b++]=tolower(c);
			break;
		case  'l': 
			Next; 
			if (c >= 'e' && c <= 'g') {	/* DOUBLE PRECISION */
				Buf[(--b)-1]=c;
				*cc='L';
			}
			else				/* INTEGER*4 */
				*cc='l';
		} 
		break;
	default : 
		while(Next!='\0' && c!='%'); 
		(*ptr)--; 
		b--; 
		*cc='%';
	}
	Buf[b]='\0'; 
	return(rtn);
}
/* get one param -- atyp = No. of words/array element (ignored if non-array) */
long onepar(atyp)
int atyp; 
{
	if(Arr && atyp && Subi>=Subz){
		Arr=0; 
		Parptr++;
	}
#ifdef D
	if(BUG)fprintf(File,"onepar: Stk[%d]+%d\n",Parptr,Arr*Subi);
#endif
	if(Arr && atyp)P.S=Stk[Parptr] + (Subi++)*atyp;
	else P.S=Stk[Parptr++];
}

#ifdef D
pribug_(n)
long *n; 
{
	BUG= *n;
}
#endif
