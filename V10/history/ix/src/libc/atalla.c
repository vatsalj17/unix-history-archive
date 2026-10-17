/* assume key is 16 hex chars */

#include <libc.h>
#include <ctype.h>

static bix();
static char rvec[10];

char *
atalla(key, chal)
	char *key, *chal;
{
	int i, res;
	char b[64], k[8];

	srand(rand()^(int)time((long*)0));
	strncpy(chal, "", 8);
	for(i=0; i<5; i++)
		chal[i] = '0'+rand()%10;
	
	if(key == 0 ) 
		return 0;
	for(i=0; i<8; i++) {
		char x[3];
		x[0] = *key++;
		x[1] = *key++;
		x[2] = 0;
		if(isxdigit(x[0])&&isxdigit(x[1]))
			;
		else
			return 0;
		sscanf(x,"%x",&k[i]);
	}

	bix(b, k);
	setkey(b);

	bix(b, chal);
	encrypt(b, 0);
	res = 0;
	for(i=0; i<32; i++)
		res = (res<<1) | b[i];
	
	sprint(rvec, "%ux", res);

	return rvec;
}

static bix(b, s)
char *b, *s;
{
	int i, j;
	for(i=0; i<8; i++)
		for(j=0; j<8; j++)
			b[8*i+j] = (s[i]>>(7-j))&1;
}
