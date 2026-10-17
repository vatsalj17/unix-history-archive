/* $Header: /ker/io.386/RCS/albaud.c,v 2.4 93/10/29 00:58:25 nigel Exp Locker: nigel $ */
/*
 * Purpose:	common code for various async drivers
 *
 * $Log:	albaud.c,v $
 * Revision 2.4  93/10/29  00:58:25  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.3  93/08/19  04:02:04  nigel
 * Nigel's R83
 * 
 * Revision 2.2  93/07/26  15:27:51  nigel
 * Nigel's R80
 * 
 * Revision 1.2  93/04/14  10:24:08  root
 * r75
 * 
 * Revision 1.1  92/04/30  08:58:54  hal
 * Add asy.  Remove silos from tty struct.
 */

#include <kernel/param.h>
#include <sys/ins8250.h>

#define TESTBAUD	0x03A5


int uart_sense();

int albaud[] ={
	0,				/* 0 */
	2304,				/* 50 */
	1536,				/* 75 */
	1047,				/* 110 */
	857,				/* 134.5 */
	768,				/* 150 */
	576,				/* 200 */
	384,				/* 300 */
	192,				/* 600 */
	96,				/* 1200 */
	64,				/* 1800 */
#ifdef _I386
	48,				/* 2400 */
	24,				/* 4800 */
	12,				/* 9600 */
	6,				/* 19200 */
	3				/* 38400 */
#else
	58,				/* 2000 */
	48,				/* 2400 */
	32,				/* 3600 */
	24,				/* 4800 */
	16,				/* 7200 */
	12,				/* 9600 */
	6,				/* 19200 */
	0,				/* EXTA */
	0				/* EXTB */
#endif
};

/*
 *	alp_rate[] is tied to albaud[] - it gives the minimum polling
 *	rate for the corresponding port speed; it must be a multiple
 *	of 100 (system clock Hz) and >= baud/6
 */

int alp_rate[] ={			/* baud/6 or zero */
	0,				/* 0 */
	1*HZ,				/* 50 */
	1*HZ,				/* 75 */
	1*HZ,				/* 110 */
	1*HZ,				/* 134.5 */
	1*HZ,				/* 150 */
	1*HZ,				/* 200 */
	1*HZ,				/* 300 */
	1*HZ,				/* 600 */
	2*HZ,				/* 1200 */
	3*HZ,				/* 1800 */
#ifdef _I386
	4*HZ,				/* 2400 */
	8*HZ,				/* 4800 */
	16*HZ,				/* 9600 */
	32*HZ,				/* 19200 */
	64*HZ				/* 38400 */
#else
	4*HZ,				/* 2000 */
	4*HZ,				/* 2400 */
	6*HZ,				/* 3600 */
	8*HZ,				/* 4800 */
	12*HZ,				/* 7200 */
	16*HZ,				/* 9600 */
	32*HZ,				/* 19200 */
	0,				/* EXTA */
	0				/* EXTB */
#endif
};

/*
 * uart_sense()
 *
 * Given port address, return what type of 8250-family chip is found there.
 *
 * 0 - no chip
 * 1 - 8250 or 8250B
 * 2 - 8250A or 16450
 * 3 - 16550
 * 4 - 16550A
 *
 * Only the last of these has usable on-chip FIFO.
 */
int uart_sense(port)
int port;
{
	int		ret = US_NONE;
	unsigned	ch;
	short		testbaud;
	char		lcr, dll, dlh;

#if 0
	/*
	 * Late in '93, apparently, Natl. Semiconductor started
	 * making 16550's that don't follow their own spec of
	 * xx00 xxxx in IIR.  So, this part of test for UART
	 * present is conditioned out, but may still be of
	 * interest.
	 */

	/*
	 * See if UART is detected at port address.
	 * UART should have IER = 0000 xxxx
	 *                  MCR = 000x xxxx
	 *                  IIR = xx00 xxxx
	 * and should be write and read back the baud rate regs.
	 */
	if (inb(port+IER) & 0xF0
	  || inb(port+MCR) & 0xE0
	  || inb(port+IIR) & 0x30) {
		goto done;
	}
#endif

	lcr = inb(port + LCR);
	outb(port+LCR, LC_DLAB);
	dll = inb(port + DLL);
	dlh = inb(port + DLH);
	outb(port+DLL, TESTBAUD & 0xFF);
	outb(port+DLH, TESTBAUD >> 8);
	testbaud = inb(port+DLL) | inb(port+DLH) << 8;
	outb(port+LCR, LC_CS8);
	if (testbaud != TESTBAUD){
		goto done;
	} else {
		outb(port+LCR, LC_DLAB);
		outb(port+DLL, dll);
		outb(port+DLH, dlh);
		outb(port+LCR, lcr);
	}

	/*
	 * Scratch register NOT found on 8250/8250B.
	 */
	outb(port+SCR, 0x55);
	ch = inb(port+SCR);
	if (ch != 0x55) {
		ret = US_8250;
	}

	/*
	 * After trying to turn on FIFO mode,
	 * If IIR is 00xx xxxx, it's 8250A/16450 (no FIFO).
	 * If IIR is 10xx xxxx, it's 16550 (broken FIFO).
	 * If IIR is 11xx xxxx, it's 16550A (usable FIFO).
	 */
	outb(port+FCR, 0x01);
	ch = inb(port+FCR);
	switch (ch & 0xC0) {
	case 0x00:
		if (ret == US_NONE)
			ret = US_16450;
		break;
	case 0x80:
		if (ret == US_NONE)
			ret = US_16550;
		break;
	case 0xC0:
		ret = US_16550A;
		break;
	}
	outb(port+FCR, 0x00);
done:

	switch(port){
	case 0x3F8:
		printf("com1 ");
		break;
	case 0x2F8:
		printf("com2 ");
		break;
	case 0x3E8:
		printf("com3 ");
		break;
	case 0x2E8:
		printf("com4 ");
		break;
	}
	printf("port %x: ", port);	
	switch (ret) {
	case US_NONE:
		printf("no UART  ");
		break;
	case US_8250:
		printf("8250/8250B  ");
		break;
	case US_16450:
		printf("8250A/16450  ");
		break;
	case US_16550:
		printf("16550 - no FIFO  ");
		break;
	case US_16550A:
		printf("16550A - FIFO  ");
		break;
	}
	return ret;
}
