/*
char id_doscan[] = "@(#)doscan.c	1.1";
 *
 * doscan:  Common code for fortran-callable formatted input routines
 * scann, fscann, sscann.
 *
 * Adapted by Bruce R. Julian, USGS, March 1980,
 * from function printn, by James W. Herriot, USGS, Feb 1980.
 *
 * Additions (by JWH) to printf format syntax are:
 *    1. %n(          where "n" is number of iterations to loop
 *    2. %na          where "n" is size of array
 *    3. %n{          shorthand for "%na %(" -- "%(" will use previous n
 *    4. %) -or- %}   end of loop
 * note that "n" above may be a constant of a "^" meaning a parameter.
 *
 * Modified by Bruce R. Julian,  USGS, Mar 1980 to:
 *     - handle double precision arrays
 *     - accept all scanf formats
 *	(Oops! Except assignment suppression.) BRJ 6 Oct 1980
 */
#define MAX  200
#include <stdio.h>
#include <ctype.h>
#include "ioprim.h"
static FILE *File;
static int  Parptr,Subi,Subz,Arr,**Stk,Nitems;
static char Buf[MAX],*Format;
static union {
	char *S; 
	char *C; 
	long *L; 
	double *D;
	int *I;
} 
P;

FORTINT doscan(farg,format,params)
FILE *farg;
char format[]; 
long *params; 
{
	File = farg;
	Parptr=Arr=0; 
	Stk= params; 
	Format=format;
	Nitems=0;
	s_recur(0); 
	return((FORTINT)Nitems);
}
s_recur(ptr)
int ptr; 
{
	int i,n,lev,o; 
	char c; 

	while( (o=s_eatstr(&ptr,&c,&n)) != -1){
		if(o) {
			for(i=0;i<n;i++)s_recur(ptr);
			lev=1; 
			while(lev+=s_eatstr(&ptr,&c,&n)); 
		}
		else{
			switch(c){
			case 's':		/* STRING */
				s_onepar(1);
				Nitems += fscanf(File,Buf, P.S); 
				break;
			case 'c':		/* CHARACTER */
				s_onepar(1);
				Nitems += fscanf(File,Buf,P.C); 
				break;
			case 'd':		/* INTEGER*2 */
			case 'o': 
			case 'x': 
				s_onepar(1);
				Nitems += fscanf(File,Buf,P.I); 
				break;
			case 'l':		/* INTEGER *4 */
				s_onepar(2);
				Nitems += fscanf(File,Buf,P.L); 
				break;
			case 'e':		/* REAL */
			case 'f': 
			case 'g': 
				s_onepar(2);
				Nitems += fscanf(File,Buf,P.D); 
				break;
			case 'L':		/* DOUBLE PRECISION */
				s_onepar(4);
				Nitems += fscanf(File, Buf, P.D);
				break;
			default:
				Nitems += fscanf(File,Buf     ); 
				break;
			}
		}
	}     
}
#define Next  (*cc=c=Buf[b++]=Format[(*ptr)++])
s_eatstr(ptr,cc,n)
int *ptr,*n; 
char *cc; 
{
	int b=0,rtn=0; 
	char c; 

	*n=0;
	switch(Next){
	case '\0': 
		(*ptr)--; 
		rtn= -1; 
		break;
	case '%': 
		while(Next=='-'||c=='.'||c>='0'&&c<='9')*n= *n*10+c-'0';
		if(c=='^'){
			s_onepar(0); 
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
		case 'E':
		case 'F':
			*cc='L';
			break;
		case  'l': 
			Next; 
			if (c == 'e' || c == 'f') 	/* DOUBLE PRECISION */
				*cc='L';
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
long s_onepar(atyp)
int atyp; 
{
	if(Arr && atyp && Subi>=Subz){
		Arr=0; 
		Parptr++;
	}
	if(Arr && atyp)P.S=Stk[Parptr] + (Subi++)*atyp;
	else P.S=Stk[Parptr++];
}
