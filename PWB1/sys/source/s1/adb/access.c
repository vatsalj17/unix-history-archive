#
/*
 *
 *	UNIX debugger
 *
 */

#include "proc.h"
#include "defs.h"


MAP		txtmap;
MAP		datmap;
INT		wtflag;
STRING		errflg;
INT		errno;

INT		devswap -1;
INT		proctab;
INT		devmem -1;

INT		pid;




/* file handling and access routines */

gtrace(mode,adr,value)
{
	INT		r;

	IF mode==RUREGS
	   ANDF chkswap()
	   ANDF seek(devswap,adr,1) != -1
	   ANDF read(devswap,&r,2)==2
	THEN	return(r);
	ELSE	return(ptrace(mode,pid,adr,value));
	FI
}

chkswap()
{
	struct proc mproc[1];

	IF devmem>=0 ANDF devswap>=0
	   ANDF seek(devmem, proctab, 0) != -1
	   ANDF read(devmem, mproc, sizeof mproc)==sizeof mproc
	   ANDF pid==mproc[0].p_pid
	   ANDF (mproc[0].p_flag&SLOAD)==0
	THEN	return(seek(devswap,mproc[0].p_addr,3)!=-1);
	ELSE	return(FALSE);
	FI
}

setproc()
{
	devswap=open(DEVSWAP,0);
	devmem=open(DEVMEM,0);
}

put(adr,space,value)
L_INT	adr;
{
	access(WT,adr,space,value);
}

POS	get(adr, space)
L_INT		adr;
{
	return(access(RD,adr,space,0));
}

POS	chkget(n, space)
L_INT		n;
{
	REG INT		w;

	w = get(n, space);
	chkerr();
	return(w);
}

access(mode,adr,space,value)
L_INT	adr;
{
	INT		w, w1, pmode, rd, file;
	rd = mode==RD;

	IF space == NSP THEN return(0); FI

	IF pid		/* tracing on? */
	THEN IF adr&01 ANDF !rd THEN error(ODDADR); FI
	     pmode = (space&DSP?(rd?RDUSER:WDUSER):(rd?RIUSER:WIUSER));
	     w = gtrace(pmode, shorten(adr), value);
	     IF adr&01
	     THEN w1 = gtrace(pmode, shorten(adr+1), value);
		  w = (w>>8)&LOBYTE | (w1<<8);
	     FI
	     IF errno
	     THEN errflg = (space&DSP ? BADDAT : BADTXT);
	     FI
	     return(w);
	FI
	w = 0;
	IF mode==WT ANDF wtflag==0
	THEN	error("not in write mode");
	FI
	IF !chkmap(&adr,space)
	THEN return(0);
	FI
	file=(space&DSP?datmap.ufd:txtmap.ufd);
	IF longseek(file,adr)==0 ORF
	   (rd ? read(file,&w,2) : write(file,&value,2)) < 1
	THEN	errflg=(space&DSP?BADDAT:BADTXT);
	FI
	return(w);

}

chkmap(adr,space)
	REG L_INT	*adr;
	REG INT		space;
{
	REG MAPPTR amap;
	amap=((space&DSP?&datmap:&txtmap));
	IF space&STAR ORF !within(*adr,amap->b1,amap->e1)
	THEN IF within(*adr,amap->b2,amap->e2)
	     THEN *adr =+ (amap->f2)-(amap->b2);
	     ELSE errflg=(space&DSP?BADDAT:BADTXT); return(0);
	     FI
	ELSE *adr =+ (amap->f1)-(amap->b1);
	FI
	return(1);
}

within(adr,lbd,ubd)
L_INT	adr, lbd, ubd;
{
	return(adr>=lbd && adr<ubd);
}

devclose()
{
	close(devmem); close(devswap);
}
