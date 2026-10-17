/* Fortran convert number to string. PLWard 3/20/80 */
#define BUFLEN  50
#define MAXPREC 40
#define EXPPLUS 1000

char numstr_(out,len,num,prec)
     char out[];
     long len;
     int *prec;
     float *num;
{
     char fmt[10];
     int precx,precy=0;
     int i,j,lenbuf;
     int dot = -1;
     char type='f';
     char buf[BUFLEN];

     precx = *prec;
     if (precx > EXPPLUS - 2 ) { precx=precx - EXPPLUS; type = 'e'; }
     if (precx < 0   ) { precy = precx; precx = 0; }
     if (precx > MAXPREC) precx = MAXPREC;

     sprintf(fmt,"%%.%d%c",precx,type);
     sprintf(buf,fmt,*num);

     for(lenbuf=0; buf[lenbuf] != '\0'; lenbuf++)
          if(buf[lenbuf] == '.' && *prec == EXPPLUS-1) dot=lenbuf;
     if (precx == 0 && type == 'f') buf[lenbuf++]='.';
     if (*prec < 0 ) lenbuf =(dot > -1) ? dot+precy+1 : lenbuf+precy;
     for(i=j=0; i<len;i++)
          if (j >= lenbuf) out[j++]=' ';
          else if ( i != dot ) out[j++]=buf[i];
}
