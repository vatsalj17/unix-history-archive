#include "ded.h"
#include "match.h"

extern char *b_re();

char *build_re(re,str,fin_ch)
struct RE re[];
char *str, fin_ch;
 { int re_count;
    char *s;

    re_count = -1;
    s = b_re(re,str,&re_count,fin_ch,false);
    re[++re_count].type = p_FIN;
    /* what am I producing */
    if (dbug('a')) print_re(re, re_count);
    return(s);
 }

/************************************************************************
 *  the routine that really does the work. Parameter names are, I hope, *
 *      self-explanatory. It eats a string, putting the result in 're'  *
 *      and returning a pointer to the tail of the string. Only called  *
 *      from build_re and recursively within itself                     *
 ************************************************************************/
char *b_re(re,str,a_count,fin_ch,special_fin)
struct RE re[];
char *str;
int *a_count;
char fin_ch;
int special_fin;
 { register int count;
    int this_count;
    register char c;
    int type, size;
    register int i;

    count = *a_count;

    while (true)
     { this_count = count+1;
	switch (c = *str++)
	 { case '\0':
		if (!special_fin)
		 { *a_count = count; return(str-1); }
		else
		 fdiag("?? '%c missing from r.e. ??", fin_ch);

	    case '\'':
		switch (c = *str++)
		 { case '[':
			*a_count = count;
			str = build_set(re,str,a_count);
			count = *a_count;
			break;

		    case '(':
			*a_count = count;
			str = b_re(re,str,a_count,')',true);
			count = *a_count;
			break;

		    case '.':
			re[++count].type = p_ANY;
			break;

		    case '^':
			re[++count].type = p_BOL;
			break;

		    case '$':
			re[++count].type = p_EOL;
			break;

		    case '\0':
			/* treat as trailing space */
			str--;
		    case ' ':
			re[++count].type = p_SEP;
			break;

		    case '?':
		    case '*':
		    case '+':
			fdiag("expression starts with '%c",c);
			break;

		    case ']':
		    case ')':
			if (special_fin && fin_ch==c)
			 { *a_count = count; return(str); }
			else
			  fdiag("?? unmatched '%c found in r.e.", c);

		    case '\'':
		    case '/':
		    case ';':
			re[++count].type = p_CHAR;
			re[count].info = c;
			break;

		    default:
			fdiag("?? invalid sequence '%c ??", c);
		 }
		break;

	    default:
		if (fin_ch==c && !special_fin)
		 { *a_count = count; return(str); }
		else
		 { re[++count].type = p_CHAR;
		    re[count].info = c;
		    break;
		 }
	 }

	/* look for '?, '*, '**, '+ and '++ (even repeated!) */
	while (*str=='\'')
	 { switch (str[1])
	     { case '*':
		    if (str[2]=='*')
		     { type = p_SSTAR; size = 3; }
		    else
		     { type = p_STAR; size = 2; }
		    break;

		case '+':
		    if (str[2]=='+')
		     { type = p_PPLUS; size = 3; }
		    else
		     { type = p_PLUS; size = 2; }
		    break;

		case '?':
		    type = p_OPT; size = 2;
		    break;

		default:
		    size = 0;
		    break;
	     }

	    if (size==0) break;
	    else str =+ size;

	    /* make what is in re shift upwards */
	    re[++count].type = p_FIN;
	    for (i = ++count; i>this_count; i--)
	     { re[i].type = re[i-1].type;
		re[i].info = re[i-1].info;
	     }

	    re[this_count].type = type;
	    re[this_count].info = count-this_count+1;
	 }

     }
 }

char *build_set(re, str, a_count)
struct RE re[];
char *str;
int *a_count;
 { register char c, c1;
    register int count;
    int initial, type;

    if (*str=='-')
     { type = p_OUTSET; str++; }
    else
      type = p_INSET;

    count = *a_count;
    re[++count].type = type;

    initial = count;

    while (true)
     { switch (c = *str++)
	 { case '\'':
		switch (c = *str++)
		 { case ']':
			re[++count].type = p_FIN;
			re[initial].info = count-initial+1;
			*a_count = count;
			return(str);

		    case '\0':
			fdiag("?? '] missing from pattern ??");

		    case '^':
			re[++count].type = p_BOL;
			break;

		    case '$':
			re[++count].type = p_EOL;
			break;

		    case '\'':
		    case '/':
		    case ';':
			re[++count].type = p_CHAR;
			re[count].info = c;
			break;

		    default:
			fdiag("?? invalid sequence '['%c ??", c);
		 }
		break;

	    case '\0':
		fdiag("?? '] missing from r.e. ??");

	    default:
		if (*str=='-')
		 { str++;
		    if ( (c1 = *str++)=='\'' || c1==0)
		      fdiag("?? character expected after dash ??");
		    else
		    if (c>c1)
		      fdiag("?? invalid sequence %c-%c ??",c,c1);
		    else
		     { re[++count].type = p_ONEOF;
			re[count].info = c;
			re[++count].type = -1;
			re[count].info = c1;
		     }
		 }
		else
		 { re[++count].type = p_CHAR;
		    re[count].info = c;
		 }
		break;
	 }
     }
 }

print_re(re, re_count)
struct RE re[];
int re_count;
 { int i;
    position(EDITROW,0); ttyreset();
    for (i = 0; i<=re_count; i++)
      printf("\n %d %d", re[i].type, re[i].info);
    printf("\n");
    editerror("");
 }
