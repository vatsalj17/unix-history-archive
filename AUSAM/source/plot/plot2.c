#include "plot.h"







/*
**
**	ALL ROUTINES HANG OFF THIS ONE,
**	IT IS CALLED STRAIGHT AFTER A FILE
**	IS OBTAINED WITH THE FLAG HANDLER ? !
**
*/
crunch()
{

	extern	yylineno;
	int	saveline;
	if( status & NO_SPECS )
	{
		getdata();
		if( !(status & NO_PLOT))
		{
			border();
			/* This is so that the scaling
			** will not change on the plotted
			** lines after the border has been
			** drawn
			*/
			status =| (XMAN_SCALE | YMAN_SCALE);
			plotdata();
		}
		return;
	}

	getoken();
	while(token != EOF)
	switch(token)
	{



		case LEGEND:
			if(tokens_found & LEGEND)
			{
				error("Legend may only occur once");
				getoken();
				nextgroup();
				break;
			}
			tokens_found =| LEGEND;
			if( !( getoken() & SUB_TOKEN))
				break;
			while((token & SUB_TOKEN) && (token != EOF))
				switch(token)
				{

				case STRING:
					if( head_count >= LINES_HEADING)
					{
						error("Too many lines of heading");
						getoken();
						break;
					}
					head_str[head_count++] = add_str(p_token);
					getoken();
					break;


				case KEY_AT:
					status =| KEY;
					getoken();
					switch(token)
					{
					case END:
						key_quadrant = END;
						getoken();
						break;

					case NUMBER:
						ykey_pos = key_quadrant = f_token;
						/* ykeypos is used because its a float */
						if(getoken() != NUMBER)
						{
							switch(key_quadrant)
							{
							case 1:
								xkey_pos = 750.0;
								ykey_pos = 700.0;
								break;
							case 2:
								xkey_pos = 200.0;
								ykey_pos = 700.0;
								break;
							case 3:
								xkey_pos = 200.0;
								ykey_pos = 210.0;
								break;
							case 4:
								xkey_pos = 750.0;
								ykey_pos = 210.0;
								break;
							default:
								error("The key quadrant can only be 1,2,3 or 4");
							}
							break;
						}
						else
						{
							xkey_pos = ykey_pos * 100.0;
							ykey_pos = f_token  * 100.0;
						}
						if((xkey_pos>1000.0) ||
							(xkey_pos<0.0) ||
							(ykey_pos>780.0)||
							(ykey_pos<0.0))

							ABORT("Key position out of range");
						getoken();
						break;
					default:
						ABORT("'end' or a number must follow 'key at'");
						break;
					}
					break;


		
				case NUMBER:
					/* IF WE FIND A NUMBER IN THIS CONTEXT,
					** THEN CHANCES ARE HE LEFT OUT 'DATA'
					** AND WE MENTION THIS EXPLICITLY
					*/
					ABORT("Number out of context. 'data' ommitted ??");


				case UNRECOGNISED:
					ABORT("Misspelt or unrecognised key word found");
				case BAD_CHAR:
					ABORT("Illegal or control character found");
				default:
					ABORT("Key word used in illegal context");
				}
				break;



		case XAXIS:
			if(tokens_found & XAXIS)
			{
				error("Xaxis may only occur once");
				getoken();
				nextgroup();
				break;
			}
			tokens_found =| XAXIS;
			getoken();
			while((token & SUB_TOKEN) && (token != EOF))
			switch(token)
			{
			case STRING:
				if(x_str)	/* AN XAXIS STRING ALREADY THERE */
					ABORT("Only 1 line of xaxis legend is allowed");
				x_str = add_str(p_token);
				getoken();
				break;

			case FROM:
				if(getoken() != NUMBER)
					ABORT("Number expected after 'from'");
				xmin = f_token;
				status =| XMAN_SCALE;
				getoken();
				break;

			case TO:
				if(getoken() != NUMBER)
					ABORT("Number expected after 'to'");
				xmax = f_token;
				getoken();
				break;

			case BY:
				if(getoken() != NUMBER)
					ABORT("Number expected after 'by'");
				xtick_gap = f_token;
				getoken();
				break;

			case AT:
				if(getoken() != NUMBER)
					ABORT("Number expected after 'at'");
				ypos = f_token;
				status =| MOVE_XAXIS;
				getoken();
					break;

			case FORMAT:
				if(getoken() != STRING)
					ABORT("String expected after 'format'");
				strcpy(xformat,p_token);
				getoken();
				break;
			case NUMBER:
				/* IF WE FIND A NUMBER IN THIS CONTEXT,
				** THEN CHANCES ARE HE LEFT OUT 'DATA'
				** AND WE MENTION THIS EXPLICITLY
				*/
				ABORT("Number out of context. 'data' ommitted ??");

			case UNRECOGNISED:
				ABORT("Misspelt or unrecognised key word found");
			case BAD_CHAR:
				ABORT("Illegal or control character found");

			default:
				ABORT("Key word used in illegal context");
			}
			break;



		case YAXIS:
			if(tokens_found & YAXIS)
			{
				error("Yaxis may only occur once");
				getoken();
				nextgroup();
				break;
			}
			tokens_found =| YAXIS;
			getoken();
			while((token & SUB_TOKEN) && (token != EOF))
			switch(token)
			{
			case STRING:
				if(y_str)	/* AN YAXIS STRING ALREADY THERE */
					ABORT("Only 1 line of yaxis legend is allowed");
				y_str = add_str(p_token);
				getoken();
				break;

			case FROM:
				if(getoken() != NUMBER)
					ABORT("Number expected after 'from'");
				ymin = f_token;
				status =| YMAN_SCALE;
				getoken();
				break;

			case TO:
				if(getoken() != NUMBER)
					ABORT("Number expected after 'to'");
				ymax = f_token;
				getoken();
				break;

			case BY:
				if(getoken() != NUMBER)
					ABORT("Number expected after 'by'");
				ytick_gap = f_token;
				getoken();
				break;

			case AT:
				if(getoken() != NUMBER)
					ABORT("Number expected after 'at'");
				xpos = f_token;
				status =| MOVE_YAXIS;
				getoken();
				break;
	
			case FORMAT:

				if(getoken() != STRING)
					ABORT("String must follow 'format'");
				strcpy(yformat,p_token);
				getoken();
				break;

			case NUMBER:
				/* IF WE FIND A NUMBER IN THIS CONTEXT,
				** THEN CHANCES ARE HE LEFT OUT 'DATA'
				** AND WE MENTION THIS EXPLICITLY
				*/
				ABORT("Number out of context. 'data' ommitted ??");

			case UNRECOGNISED:
				ABORT("Misspelt or unrecognised key word found");
			case BAD_CHAR:
				ABORT("Illegal or control character found");

			default:
				ABORT("Key word used in illegal context");
			}
			break;



		case DATE:
			if(tokens_found & DATE)
			{
				error("Date may only occur once");
				getoken();
				nextgroup();
				break;
			}
			if(getoken() == STRING)
			{
				date_str = add_str(p_token);
				getoken();
				break;
			}
			if( ! (token & SUB_TOKEN))	/* ITS A MAIN TOKEN */
			{
				date_str = 0;
				break;
			}
			ABORT("Only a string may follow 'date'");
			/* IF WE GET TO HERE THERE IS AN ERROR	*/



		case PLOT:
			if(tokens_found & PLOT)
			{
				error("'Plot' must only occur once per set of data");
				getoken();
				nextgroup();
				break;
			}
			tokens_found =| PLOT;
			getoken();
			while((token & SUB_TOKEN) && (token != EOF))
				switch(token)
				{

				case STRING:
					if(label_str)
						ABORT("Only 1 string may label a set of data");
					label_str = add_str(p_token);
					getoken();
					break;

				case POINT_EQ:
					switch(getoken())
					{
					case CROSS:
					case DOT:
					case STAR:
					case CIRCLE:
					case NOTHING:
					case OTHER:
						point_type = token;
						/*
						** BELOW IS A FIX TO A BUG.
						** VIS, IF NO POINT TYPE IS
						** GIVEN THEN NOTHING IS DEFAULT
						** UNLESS HE IS DOING A REGRESSION
						** WHEN CROSSES BECOME DEFAULT.
						** THUS REG WILL USE CROSSES UNLESS
						** THIS FLAG IS SET, WHEN IT WILL
						** USE THE GIVEN POINT TYPE
						** INSTEAD.
						*/
						gstatus =| POINT_EXPLICIT;
						getoken();
						break;
					default:
						ABORT("Illegal symbol after 'point'");
					}
					break;

				case LINE_EQ:
					gstatus =| LINE_EXPLICIT;
					switch(getoken())
					{
					case SOLID:
					case SHORT:
					case LONG:
					case SHORT_LONG:
					case NOTHING:
					case OTHER:
						line_type = token;
						getoken();
						break;

					default:
						ABORT("Illegal symbol after 'line='");
					}
					break;

				case X_EQ:
					if(getoken() != NUMBER)
						ABORT("A number must follow 'x='");
					x_column = f_token;
					if( ! (getoken() & SUB_TOKEN))
						break; /* get out if main token */
					switch(token)
					{
					case TO:
						if( status & YINC )
							ABORT("X and Y cannot both be auto incremented");
						inc_min = x_column;
						status =| XINC;
						if(getoken() != NUMBER)
							ABORT("Number expected after 'to'");
						inc_max = f_token;
						getoken();
						break;

					case TIMES:
						if(getoken() != NUMBER)
							ABORT("Number expected after '*'");
						xscale = f_token;
						getoken();
						break;

					default:
						break;
					}
					break;



				case Y_EQ:
					if(getoken() != NUMBER)
						ABORT("A number must follow 'y='");
					y_column = f_token;
					if( ! (getoken() & SUB_TOKEN))
						break; /* get out if main token */
					switch(token)
					{
					case TO:
						if(status & XINC)
							ABORT("X and Y cannot both be auto incremented");
						inc_min = y_column;
						status =| YINC;
						if(getoken() != NUMBER)
							ABORT("Number expected after 'to'");
						inc_max = f_token;
						getoken();
						break;

					case TIMES:
						if(getoken() != NUMBER)
							ABORT("Number expected after '*'");
						yscale = f_token;
						getoken();
						break;

					default:
						break;
					}
					break;



				case REGRESSION:
					status =| REG;
					if(getoken() != NUMBER)
					{
						reg_quadrant = NO;
						break;
					}
					yreg_pos = reg_quadrant = f_token;
					if(getoken() == NUMBER)
					{
						xreg_pos = yreg_pos * 100.0;
						yreg_pos = f_token * 100.0;
						if((xreg_pos>1000.0) ||
							(xreg_pos<0.0) ||
							(yreg_pos>780.0)||
							(yreg_pos<0.0))
							ABORT("Position of regression stats out of range");
						getoken();
					}
					else
						switch(reg_quadrant)
						{
						case 1:
							xreg_pos = 800.0;
							yreg_pos = 700.0;
							break;
						case 2:
							xreg_pos = 200.0;
							yreg_pos = 700.0;
							break;
						case 3:
							xreg_pos = 200.0;
							yreg_pos = 125.0;
							break;
						case 4:
							xreg_pos = 800.0;
							yreg_pos = 125.0;
							break;
						default:
							ABORT("The regression quadrant must be 1,2,3 or 4");
						}
					break;


		
				case NUMBER:
					/* IF WE FIND A NUMBER IN THIS CONTEXT,
					** THEN CHANCES ARE HE LEFT OUT 'DATA'
					** AND WE MENTION THIS EXPLICITLY
					*/
					ABORT("Number out of context. 'data' ommitted ??");
				case UNRECOGNISED:
					ABORT("Misspelt or unrecognised key word found");
				case BAD_CHAR:
					ABORT("Illegal or control character found");
				default:
					ABORT("Key word used in illegal context");
				}
				break;



		case MARK:
			if(tokens_found & MARK)
			{
				error("'Mark' must only occur once per set of data");
				getoken();
				nextgroup();
				break;
			}
			tokens_found =| MARK;
			getoken();
			while((token & SUB_TOKEN) && (token != EOF))
				switch(token)
				{

				case X_EQ:
					if(getoken() != NUMBER)
						ABORT("A number must follow 'x='");
					if(xmark_count == NUM_MARKS-1)
						ABORT("Too many x='s after 'mark'");
					xmark[xmark_count++] = f_token;
					gstatus =| XMARK;
					getoken();
					break;

				case Y_EQ:
					if(getoken() != NUMBER )
						ABORT("A number must follow 'y='");
					if(ymark_count == NUM_MARKS-1)
						ABORT("Too many y='s after 'mark'");
					ymark[ymark_count++] = f_token;
					gstatus =| YMARK;
					getoken();
					break;
				case LINE_EQ:
					switch(getoken())
					{
					case SOLID:
					case SHORT:
					case LONG:
					case SHORT_LONG:
					case NOTHING:
					case OTHER:
						mark_type = token;
						getoken();
						break;
					default:
						ABORT("Illegal symbol after 'line='");
					}
					break;


				case NUMBER:
					/* IF WE FIND A NUMBER IN THIS CONTEXT,
					** THEN CHANCES ARE HE LEFT OUT 'DATA'
					** AND WE MENTION THIS EXPLICITLY
					*/
					ABORT("Number out of context. 'data' ommitted ??");

				case UNRECOGNISED:
					ABORT("Misspelt or unrecognised key word found");
				case BAD_CHAR:
					ABORT("Illegal or control character found");

				default:
					if(token & SUB_TOKEN)
						ABORT("Illegal key word after 'mark'");
				}
				break;


		case NEXT:
			if(getoken() == STRING)
			{
				got_some_data = NULL;
				save_stdin(p_token);
#ifdef	DEBUG
				dump("About to recurse after 'next' found");
#endif
				crunch();
#ifdef	DEBUG
				dump("About to switch back files after recursion");
#endif
				restore_stdin();
#ifdef	DEBUG
				dump("Now after restoring input file");
#endif
				getoken();
			}
			else
				ABORT("A file name must follow 'next'");
			break;
		case DATA:
			/*
			** This saveline stuff is a bit cludgy
			** But stops certain undetected errors
			** from occuring
			*/
			saveline = yylineno;
			if((getoken() == STRING) || (yylineno == saveline))
			{
				/* SAVE CURRENT STDIN ETC */
				got_some_data = NULL;
				save_stdin(p_token);
				getdata();
				if( !(status & NO_PLOT))
				{
					border();
					/* This is so that the scaling
					** will not change on the plotted
					** lines after the border has been
					** drawn
					*/
					status =| (XMAN_SCALE | YMAN_SCALE);
					plotdata();
				}
				restore_stdin();
				label_str = 0;
				reset();
				getoken();
				break;
			}
			/*
			** IF THERE IS NO DATA AT ALL:
			** IE EOF, THEN USE THE NEXT FILE
			** IN THE ARGUMENT LIST AS THE DATA
			** FILE. IF THERE IS NO NEXT ARGUMENT
			** THEN USE STANDARD INPUT.
			** BUT UNFORTUNATELY THIS CAN ONLY
			** BE DONE BY PUTTING A '-' OR NULL FLAG
			** AT THE END OF THE COMMAND LINE
			*/

			if(token == EOF)
			{
				status =| NO_SPECS;
				return;
			}
			got_some_data = p_token;	/* WE HAVE READ SOME
			POTENTIAL DATA AND A POINTER TO IT IS SET UP SO
			GETDATA WILL NOT BOMB ON THE FIRST LINE BECAUSE
			THE FIRST CHUNK IS MISSING	*/

			token = EOF;
			/* THE ABOVE IS BECAUSE GETDATA BYPASSES
			** LEX AND IT WILL NOT FIND THE EOF THAT
			** GETDATA FOUND STRAIGHT AWAY WHEN IT RESTARTS
			*/
			getdata();
			if( !(status & NO_PLOT))
			{
				border();
				/* This is so that the scaling
				** will not change on the plotted
				** lines after the border has been
				** drawn
				*/
				status =| (XMAN_SCALE | YMAN_SCALE);
				plotdata();
			}
			break;


		case EOF:
			break;


		case NUMBER:
			/* IF WE FIND A NUMBER IN THIS CONTEXT,
			** THEN CHANCES ARE HE LEFT OUT 'DATA'
			** AND WE MENTION THIS EXPLICITLY
			*/
			ABORT("Number out of context. 'data' ommitted ??");
		case UNRECOGNISED:
			ABORT("Misspelt or unrecognised key word found");
		case BAD_CHAR:
			ABORT("Illegal or control character found");
		default:
			ABORT("Key word used in illegal context");
	}
}
