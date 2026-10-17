/* Some V9 stuff */

#define getrect3()	getrectb(1)
Rectangle	getrectb();
#define qpeekc(Q)	(unsigned char)(Q.c_cc?(*(Q.first)):0)

#define Texture Texture16

#define	RESEARCH
