/*
 * Set up software prototype segmentation
 * registers to implement the 3 pseudo
 * text,data,stack segment sizes passed
 * as arguments.
 * The argument sep specifies if the
 * text and data+stack segments are to
 * be separated.
 */
estabur(nt, nd, ns, sep)
{
	register a, *ap, *dp;

	if(sep) {
#ifndef	_1140
		if(cputype == 40)
			goto err;
		if(nseg(nt) > 8 || nseg(nd)+nseg(ns) > 8)
#endif
			goto err;
	} else
		if(nseg(nt)+nseg(nd)+nseg(ns) > 8)
			goto err;
	if(nt+nd+ns+USIZE > maxmem)
		goto err;
	a = 0;
	ap = &u.u_uisa[0];
	dp = &u.u_uisd[0];
	while(nt >= 128) {
		*dp++ = (127<<8) | RO;
		*ap++ = a;
		a =+ 128;
		nt =- 128;
	}
	if(nt) {
		*dp++ = ((nt-1)<<8) | RO;
		*ap++ = a;
	}
#ifndef	_1140
	if(sep)
	while(ap < &u.u_uisa[8]) {
		*ap++ = 0;
		*dp++ = 0;
	}
#endif
	a = USIZE;
	while(nd >= 128) {
		*dp++ = (127<<8) | RW;
		*ap++ = a;
		a =+ 128;
		nd =- 128;
	}
	if(nd) {
		*dp++ = ((nd-1)<<8) | RW;
		*ap++ = a;
		a =+ nd;
	}
	while(ap < &u.u_uisa[8]) {
		*dp++ = 0;
		*ap++ = 0;
	}
#ifndef	_1140
	if(sep)
	while(ap < &u.u_uisa[16]) {
		*dp++ = 0;
		*ap++ = 0;
	}
#endif
	a =+ ns;
	while(ns >= 128) {
		a =- 128;
		ns =- 128;
		*--dp = (127<<8) | RW;
		*--ap = a;
	}
	if(ns) {
		*--dp = ((128-ns)<<8) | RW | ED;
		*--ap = a-128;
	}
#ifndef	_1140
	if(!sep)
	{
		ap = &u.u_uisa[0];
		dp = &u.u_uisa[8];
		while(ap < &u.u_uisa[8])
			*dp++ = *ap++;
		ap = &u.u_uisd[0];
		dp = &u.u_uisd[8];
		while(ap < &u.u_uisd[8])
			*dp++ = *ap++;
	}
#endif
	sureg();
	return(0);

err:
	u.u_error = ENOMEM;
	return(-1);
}

/*
 * Load the user hardware segmentation
 * registers from the software prototype.
 * The software registers must have
 * been setup prior by estabur.
 */

#ifndef _1140 | _1170 | _1145
sureg()
{
	register *up, *rp, a;

	a = u.u_procp->p_addr;
	up = &u.u_uisa[16];
	rp = &UISA->r[16];
	if(cputype == 40) {
		up =- 8;
		rp =- 8;
	}
	while(rp > &UISA->r[0])
		*--rp = *--up + a;
	if((up=u.u_procp->p_textp) != NULL)
		a =- up->x_caddr;
	up = &u.u_uisd[16];
	rp = &UISD->r[16];
	if(cputype == 40) {
		up =- 8;
		rp =- 8;
	}
	while(rp > &UISD->r[0]) {
		*--rp = *--up;
		if((*rp & WO) == 0) {
			rp[(UISA-UISD)/2] =- a;
#ifdef	SHARED_DATA
			if( u.u_flags & USHRDATA ) *rp =| RW;
#endif
		}
	}
}
#endif

#ifdef	_1140 | _1170 | _1145
sureg()
{
	register *up, *rp, a;

	a = u.u_procp->p_addr;
#ifdef	_1140
	up = &u.u_uisa[8];
	rp = &UISA->r[8];
#endif
#ifndef	_1140
	up = &u.u_uisa[16];
	rp = &UISA->r[16];
#endif
	while(rp > &UISA->r[0])
		*--rp = *--up + a;
	if((up=u.u_procp->p_textp) != NULL)
		a =- up->x_caddr;
#ifdef	_1140
	up = &u.u_uisd[8];
	rp = &UISD->r[8];
#endif
#ifndef	_1140
	up = &u.u_uisd[16];
	rp = &UISD->r[16];
#endif
	while(rp > &UISD->r[0]) {
		*--rp = *--up;
		if((*rp & WO) == 0) {
			rp[(UISA-UISD)/2] =- a;
#ifdef	SHARED_DATA
			if( u.u_flags & USHRDATA ) *rp =| RW;
#endif
		}
	}
}
#endif

/*
 * Return the arg/128 rounded up.
 */
nseg(n)
{

	return((n+127)>>7);
}
