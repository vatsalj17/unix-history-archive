#include <stdio.h>

/* Deroff command -- strip troff, eqn, and Tbl sequences from
a file.  Has one flag argument, -w, to cause output one word per line
rather than in the original format.
Deroff follows .so and .nx commands, removes contents of macro
definitions, equations (both .EQ ... .EN and $...$),
Tbl command sequences, and Troff backslash constructions.

All input is through the C macro; the most recently read character is in c.
*/

#define C ( (c=getc(infile)) == EOF ? eof() : (c==ldelim ? skeqn() : c) )
#define SKIP while(C != '\n') 
#define NOCHAR -2
#define SPECIAL 0
#define APOS 1
#define DIGIT 2
#define LETTER 3

int wordflag;

char chars[128];  /* SPECIAL, APOS, DIGIT, or LETTER */

char line[512];
char *lp;

int c;
int ldelim NOCHAR;
int rdelim NOCHAR;

int intable 0;

int argc;
char **argv;

char fname[50];
FILE *files[15];
FILE **filesp;
FILE *infile;



main(ac, av)
int ac;
char **av;
{
int i;
char *p;

argc = ac - 1;
argv = av + 1;

while(argc>0 && argv[0][0]=='-' && argv[0][1]!='\0') 
	{
	for(p=argv[0]+1; *p; ++p) switch(*p)
		{
		case 'w':
			++wordflag;
			break;
		default:
			fatal("Invalid flag %c\n", *p);
		}
	--argc;
	++argv;
	}

if(argc == 0)
	infile = stdin;
else	{
	infile = opn(argv[0]);
	--argc;
	++argv;
	}

files[0] = infile;
filesp = &files[0];

for(i='a'; i<='z' ; ++i)
	chars[i] = LETTER;
for(i='A'; i<='Z'; ++i)
	chars[i] = LETTER;
for(i='0'; i<='9'; ++i)
	chars[i] = DIGIT;
chars['\''] = APOS;
chars['&'] = APOS;

work();
}



skeqn()
{
while((c = getc(infile)) != rdelim)
	if(c == EOF) c = eof();
return(C);
}


opn(p)
char *p;
{
FILE *fd;

if(p[0]=='-' && p[1]=='\0')
	fd = stdin;
else if( (fd = fopen(p, "r")) == NULL)
	fatal("Cannot open file %s\n", p);

return(fd);
}



eof()
{
if(infile != stdin)
	fclose(infile);
if(filesp > files)
	infile = *--filesp;
else if(argc > 0)
	{
	infile = opn(argv[0]);
	--argc;
	++argv;
	}
else
	exit(0);

return(C);
}



getfname()
{
register char *p;
struct chain { struct chain *nextp; char *datap; } *chainblock;
struct chain *q;
static struct chain *namechain 0;

while(C == ' ') ;

for(p = fname ; (*p=c)!= '\n' ; ++p)
	C;
*p = '\0';

/* see if this name has already been used */

for(q = namechain ; q; q = q->nextp)
	if(equals(fname, q->datap))
		{
		fname[0] = '\0';
		return;
		}

q = calloc(1,sizeof(*chainblock));
q->nextp = namechain;
q->datap = copys(fname);
namechain = q;
}




fatal(s,p)
char *s, *p;
{
fprintf(stderr, "Deroff: ");
fprintf(stderr, s, p);
exit(1);
}

work()
{

for( ;; )
	{
	if(C == '.'  ||  c == '\'')
		comline();

	else	{
		line[0] = c;
		lp = line;
		for( ; ; )
			{
			if(c == '\\')
				{
				*lp = ' ';
				backsl();
				}
			if(c == '\n') break;
			if(intable && c=='T')
				{
				*++lp = C;
				if(c=='{' || c=='}')
					{
					lp[-1] = ' ';
					*lp = C;
					}
				}
			else	*++lp = C;
			}
		
		*lp = '\0';
		
		if(line[0] != '\0')
			if(wordflag == 0)
				puts(line);
			else putwords();
		
		}
	}
}



putwords()	/* break into words for -w option */
{
register char *p, *p1;
int i, nlet;


for(p1 = line ; ;)
	{
	/* skip initial specials ampersands and apostrophes */
	while( chars[*p1] < DIGIT)
		if(*p1++ == '\0') return;
	nlet = 0;
	for(p = p1 ; (i=chars[*p]) != SPECIAL ; ++p)
		if(i == LETTER) ++nlet;

	if(nlet < 2)   /* MDM definition of word */
		p1 = p;
	else	{
		/* delete trailing ampersands and apostrophes */
		while(p[-1]=='\'' || p[-1]=='&')
			 --p;
		while(p1 < p) putchar(*p1++);
		putchar('\n');
		}
	}
}



comline()
{
register int c1, c2;

while(C==' ' || c=='\t')
	;
if( (c1=c)=='\n' || (c2=C)=='\n') return;

if(c1=='E' && c2=='Q')
	eqn();
else if(c1=='T' && (c2=='S' || c2=='C' || c2=='&') )
	tbl();
else if(c1=='T' && c2=='E')
	intable = 0;
else if(c1=='d' && c2=='e')
	macro();
else if(c1=='i' && c2=='g')
	macro();
else if(c1=='a' && c2 == 'm')
	macro();
else if(c1=='s' && c2=='o')
	{
	getfname();
	if( fname[0] )
		infile = *++filesp = opn( fname );
	}
else if(c1=='n' && c2=='x')
	{
	getfname();
	if(fname[0] == '\0') exit(0);
	if(infile != stdin)
		fclose(infile);
	infile = *filesp = opn(fname);
	}
else SKIP;
}



macro()
{
do { SKIP; }
	while(C!='.' || C!='.');	/* look for  .EN */
SKIP;
}




tbl()
{
while(C != '.');
SKIP;
intable = 1;
}

eqn()
{
int c1, c2;

SKIP;

for( ;;)
	{
	if(C == '.'  || c == '\'')
		{
		while(C==' ' || c=='\t')
			;
		if(c=='E' && C=='N')
			{
			SKIP;
			return;
			}
		}
	else if(c == 'd')	/* look for delim */
		{
		if(C=='e' && C=='l'
		&& C=='i' && C=='m')
			{
			while(C == ' ');
			if((c1=c)=='\n' || (c2=C)=='\n'
			    || (c1=='o' && c2=='f' && C=='f') )
				{
				ldelim = NOCHAR;
				rdelim = NOCHAR;
				}
			else	{
				ldelim = c1;
				rdelim = c2;
				}
			}
		}

	skip:	if(c != '\n')  SKIP;
	}
}



backsl()	/* skip over a complete backslash construction */
{
int bdelim;

switch(C)
	{
	case 's':
		if(C == '\\') backsl();
		else	{
			while(C>='0' && c<='9') ;
			ungetc(c,infile);
			c = '0';
			}
		--lp;
		return;

	case 'f':
	case 'n':
	case '*':
		if(C != '(')
			return;

	case '(':
		if(C != '\n') C;
		return;

	case '$':
		return;

	case 'b':
	case 'x':
	case 'v':
	case 'h':
	case 'w':
	case 'o':
	case 'l':
	case 'L':
		if( (bdelim=C) == '\n')
			return;
		while(C!='\n' && c!=bdelim)
			if(c == '\\') backsl();
		return;

	default:
		return;
	}
}




copys(s)
register char *s;
{
register char *t;
int k;

for(t=s; *t++ ; );
if( (k = calloc( t-s , sizeof(*t))) == NULL)
	fatal("Cannot allocate memory");

for(t=k ; *t++ = *s++ ; );
return(k);
}



equals(a,b)
register char *a,*b;
{
if(a==b) return(1);

while(*a == *b)
	if(*a == '\0') return(1);
	else {++a; ++b;}

return(0);
}
