#include	"mac.h"
#include	"mac.x"


int	next;			/* next state in the automaton */
char	*opr;			/* pointer to temp code */

pass1()
{
	register struct fd *f;		/* fmt descr */
	register int sret;		/* success return */
	register int fret;		/* failure return */
	register int s;
	register int i;
	register char *r;		/* for opcode table */


	eof = FALSE;
	newline();			/* general initialisation */

	/*
	 *   Start the automaton.  Once it is going, -
	 *   it keeps itself going until end-of-input.
	 */
	while (!eof)  {


		s = parse[next].tb_sym;


		/*
		 *   If no match or sym != MCH (always match)
		 *   proceed to next case in parser table.
		 */
		if (sym != (s < 0  ?  -s : s)  &&  s != MCH)  {
			next++;
			continue;
			}

		/*
		 *   Have a sym match.
		 *   If sym < 0 this implies that the member
		 *   must be checked as well !.
		 *   Failure to match can occur here as well.
		 */
		if (s < 0  &&  mem != parse[next].tb_mem)  {
			next++;
			continue;
			}

		/*
		 *   Sym (and mem) match -
		 *   perform action specified.
		 */
		switch (parse[next].tb_act)  {


				/* no operation */
			case NOOP:
				break;


				/* add label tag */
			case ALBL:
				i = locn[lcntr].l_value;
				i = lscan(clabel, DEF, mem, i, REL);
				if (i == ERR)  {
					newline();
					continue;
					}
				intercode.i_label = i;
				break;


				/* decode opcode value */
			case DOPV:
				i = pscan();		/* pseudo ? */
				if (i != ERR)  {
					intercode.i_flags = PS;
					intercode.i_op = i;
					fn = pradr[i];
					break;
					}

				i = oscan();		/* opcode ? */
				if (i == ERR)  {
					synerr("op not found");
					newline();
					continue;
					}

				intercode.i_flags = OP;
				intercode.i_op = i;
				break;


				/* out operand label */
			case OLBL:
				i = lscan(clabel, LKP, mem, NUL, NUL);
				*opr++ = '$';
				opr = num(i, opr);
				break;


				/* out operand operator */
			case OOPR:
				*opr++ = oprtab[mem];
				break;


				/* out operand delimiter */
			case ODEL:
				*opr++ = ',';
				break;


				/* out operand constant */
			case OCON:
				*opr++ = '#';
				opr = num(mem, opr);
				break;


				/* out extra character */
			case OCHR:
				*opr++ = mem;
				break;


				/* out string in "'s */
			case OSTR:
				*opr++ = '"';
				for (i=0; i<mem; i++)
					*opr++ = clabel[i];
				*opr++ = '"';
				break;


				/* end pass 1 */
			case ENDP:
				warning("no end stmt");
				prend();
				return;


				/* select options */
			case SELC:
				for (i=0; i<4; i++)
					intercode.i_selc[i] = parse[next].tb_arg[i];
				next = parse[next].tb_next;
				continue;


				/* call expr parser */
			case EXPR:
				fret = next + 1;
				sret = parse[next].tb_next;
				next = 0;		/* start of expr */
				continue;


				/* good expr return */
			case RETN:
				next = sret;
				sret = NUL;
				continue;


				/* fail expr return */
			case GOTO:
				next = fret;
				fret = NUL;
				continue;


				/* input line error */
			case OERR:
				synerr("syntax error");
				newline();
				continue;


				/* scan ok - write intercode */
			case OREC:
				*opr = '\0';
				write(unit, &intercode, IT);

				if (intercode.i_flags & OP)  {
					r = &opcode[0] + (intercode.i_op * head.h_o_len);
					if (intercode.i_selc[0] & SELOPC)
						/* select opcode column */
						i = intercode.i_selc[2];
	
					else
						/* select default */
						i = 0;
	
					if (intercode.i_selc[0] & SELFMT)
						/* select new format */
						f = &memory[intercode.i_selc[3]];
	
					else
						/* select default */
						f = &memory[r->o_code[i].o_format];

					/* increment pc */
					locn[lcntr].l_value =+ f->f_len;
					}


				if (intercode.i_flags & PS)
					/* call pseudo routine */
					(*fn)();


				newline();
				continue;



				/* bad action - fatal */
			default:
				synerr("pass 1 non-existant action");
				exit(1);

			}


		/*
		 *   Successful match - get new input symbol
		 *   and move to next automaton state.
		 */
		getsym();
		next = parse[next].tb_next;

		}

	/*
	 *   end of pass 1
	 */
}


newline()
{
	register char *r;
	register int i;

	if (eof)
		return;

	getlin();
	getsym();

	nline++;
	intercode.i_op = intercode.i_label = ERR;
	intercode.i_flags = NUL;
	intercode.i_loc = lcntr;
	intercode.i_selc[0] = NUL;		/* no actions */
	opr = r = &intercode.i_opr[0];
	for (i=0; i<32; i++)
		/* clear intercode argument buffer */
		*r++ = '\0';

	next = head.h_p_start;

	return;
}
