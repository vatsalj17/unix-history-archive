/*
 * Modified to allow loading and booting of I space
 * only unix v7 monitors. The variable "long phys"
 * in the copyunix function was changed to "unsigned phys_i"
 * This change permitted "case 407" to load files with
 * a text+data segment size of greater than 32767 bytes.
 * Prior to the change the code would load the file
 * successfuly but would hang in the for loop, because
 * of the way "addr" and "phys" were compared.
 *
 * Modified for use with the overlay unix kernel.
 *
 * Fred Canter 7/28/81
 */
#include "../h/param.h"
#include "../h/ino.h"
#include "../h/inode.h"
#include "../h/filsys.h"
#include "../h/dir.h"
#include "../h/seg.h"
#include "saio.h"

#define KDSA6 ((physadr)0172374)
#define	KISA6 ((physadr)0172354)

/*
 * "cpuid" is set up by M.s and indicates the class
 * of CPU being used, i.e., seperate I & D space
 * or I space only.
 *
 * cpuid = 0, for separate I & D space CPU's.
 * cpuid = 1, for non-separate I & D space CPU's.
 *
 */

int	cpuid;
char line[100];

main()
{
int i;

/*
 * Set the segflag for a 64k word boundry.
 */
	segflag = 2;

boot:
	printf("Boot\n");
	do {
		printf(": "); gets(line);
		i = open(line,0);
	} while (i < 0);


	if (copyunix(i))
		goto boot;

}


copyunix(io)
register io;
{
register addr,s;
long phys;
long ovaddr;
unsigned phys_i;
unsigned	txtsiz,datsiz,bsssiz,ovsize;
int	magic;
unsigned ovsizes[8];
int i;


	lseek(io, (off_t)0, 0);
	magic = getw(io);
	txtsiz = getw(io);
	datsiz = getw(io);
	bsssiz = getw(io);


	switch (magic) {
	case 0411:
		/*
		 * Print an error message if loading of a seperate
		 * I and D space file is attempted on an I space
		 * only CPU, such as 11/40.
		 */
		if(cpuid) {
			printf("\nCan't load sep I&D ( %o) files\n", magic);
			close(io);
			return(1);
			}
		/*
		 * When loading a seperate I and D space file,
		 * move the kernel stack to the last 8 kb
		 * section of the first 128 kb of memory.
		 * This prevents the stack from overwriting unix.
		 */
		KDSA6->r[0] = 03600;
		ssid();		/* Insure sep I & D space enabled */
		setseg(0);
		lseek(io, (long)(020+txtsiz), 0);

		for(addr=0; addr!=datsiz; addr+=2)  {
			mtpi(getw(io),addr);
		}

		clrseg(addr,bsssiz);

		phys = (long)datsiz + (long)bsssiz + 63L;
		phys =/ 64;
		setseg((int)phys);

		lseek(io, 020L, 0);

		for(addr=0; addr!=txtsiz; addr+=2) {
			mtpi(getw(io),addr);
		}
		close(io);
		return(0);
	case 0407:
		/*
		 * If loading a 407 type monitor on a separate I & D space
		 * type CPU, disable separate I & D space.
		 */
		if(!cpuid) {
			setseg(0);
			snsid();	/* disable sep I & D space */
			}
		/*
		 * space over the header. We do this instead of seeking
		 * because the input might be a tape which doesn't know 
		 * how to seek.
		 */
		getw(io); getw(io); getw(io); getw(io);
		phys_i = txtsiz+datsiz;
		for (addr = 0; addr != phys_i; addr += 2)
			mtpi(getw(io),addr);
		clrseg(addr, bsssiz);
		close(io);
		return(0);

	case 0430:	/* overlayed text */
		/*
		 * If loading an overlay text kernel on a separate
		 * I & D space CPU, disable separate I & D space.
		 */
		if (!cpuid)
			snsid();
		
		/*
		 * check for consistency
		 */
		if (txtsiz < 017777) {
			printf("text segment too small (0%o)\n", txtsiz);
			close(io);
			return(1);
		}
		if (txtsiz > 037777) {
			printf("text segment too big (0%o)\n", txtsiz);
			close(io);
			return(1);
		}
		if (datsiz + bsssiz > 057777) {
			printf("data segment too big (0%o)\n", datsiz + bsssiz);
			close(io);
			return(1);
		}

		lseek(io, (long) 020, 0);	/* skip to overlay header */
		for (i = 0, ovsize = 0; i < 8; i++) {
			ovsizes[i] = getw(io);
			ovsize += ovsizes[i];
		}
		ovsize -= ovsizes[0];
		if (ovsizes[0] > 017777) {
			printf("max overlay too big (0%o)\n", ovsizes[0]);
			close(io);
			return(1);
		}
		ovsizes[0] = 0;
		/*
		 * load text segment at zero
		 */
		setseg(0);
		for (addr = 0; addr < txtsiz; addr +=2)
			mtpi(getw(io), addr);
		/*
		 * load data segment at 24Kb
		 */
		phys = 060000L;
		setseg((int)(phys/64));
		lseek(io, 040L + (long)txtsiz + (long)ovsize, 0);
		for (addr = 0; addr < datsiz; addr += 2)
			mtpi(getw(io), addr);
		/*
		 * clear bss
		 */
		clrseg(addr, bsssiz);
		phys += datsiz + bsssiz;
		phys = (phys + 077) & ~077;
		/*
		 * load the overlays after the bss
		 * except for overlay 1, which is loaded at 16Kb
		 */
		lseek(io, (long)(040+txtsiz), 0);
		ovaddr = phys;
		for (i = 1; i < 8; i++) {
			if (i == 1)
				setseg(040000/64);
			else {
				setseg((int)(phys/64));
			/*
			 * When loading an overlay text kernel,
			 * move the kernel stack to the last 8 kb
			 * section of the first 128 kb of memory.
			 * This prevents the stack from overwriting unix.
			 */
				KISA6->r[0] = 03600;
				phys += ovsizes[i];
			}
			for (addr = 0; addr < ovsizes[i]; addr += 2)
				mtpi(getw(io), addr);
		}

		/*
		 * build the overlay address and descriptor
		 * tables for the kernel.
		 */
		setseg(0);
		/*
		 * When loading an overlay text kernel,
		 * move the kernel stack to the last 8 kb
		 * section of the first 128 kb of memory.
		 * This prevents the stack from overwriting unix.
		 */
		KISA6->r[0] = 03600;
		addr = 01004;	/* overlay tables start here */
		for (i = 0; i < 8; i++) {
			if (i == 1)
				mtpi(040000/64, addr);
			else {
				mtpi((int)(ovaddr/64), addr);
				ovaddr += ovsizes[i];
			}
			addr += 2;
		}
		for (i = 0; i < 8; i++) {
			if (ovsizes[i] != 0)
				mtpi((((ovsizes[i]/64)-1)<<8) | RO, addr);
			else
				mtpi(0, addr);
			addr += 2;
		}
		mtpi((int)(phys/64), addr);	/* start of free memory */
/*
 * Allthough separate I & D space was disabled above,
 * must call snsid() again to reset KISA7 to map to
 * I/O space, because setseg(); changes it.
 */
		if(!cpuid)
			snsid();
		close(io);
		return(0);

	default:
		printf("Can't load %o files\n", magic);
		close(io);
		return(1);
	}
}
