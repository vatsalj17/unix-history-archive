/*
 *	Interdata Fortran V error messages
 */

main(argc, argv)
char *argv[];
{
	register i;

	if (argc < 2) {
		printf("Usage: ferr error-number ...\n");
		exit();
	}
	for (i=1; i<argc; i++) {
		printf(ferr(atoi(argv[i])));
		putchar('\n');
	}
}
ferr(n)
{
switch (n) {
case 1: return("ISIGN: Argument 1 = 0 and argument 2 < 0");
case 2: return("SIGN: Argument 1 = 0 and argument 2 < 0");
case 3: return(".Y: Overflow");
case 4: return(".I: Overflow");
case 5: return(".I: Power < 0");
case 6: return(".A: Base < 0 and fractional exponent.");
case 7: return(".E: Base < 0 and fractional exponent.");
case 8: return(".B: Base < 0 and fractional exponent.");
case 9: return(".F: Base < 0 and fractional exponent.");
case 10: return("SQRT: Base < 0");
case 11: return("DSQRT: Base < 0");
case 12: return("ALOG10: Argument < 0");
case 13: return("DLOG10: Argument = 0");
case 14: return("DLOG10: Argument < 0");
case 15: return("ALOG10: Argument = 0");
case 16: return("ALOG: Argument < 0");
case 17: return("ALOG: Argument = 0");
case 18: return("DLOG: Argument < 0");
case 19: return("DLOG: Argument = 0");
case 20: return("EXP: Overflow");
case 21: return("EXP: Underflow");
case 22: return("DEXP: Overflow");
case 23: return("DEXP: Underflow");
case 24: return("@A: Overflow");
case 25: return("@A: Underflow");
case 26: return("@S: Overflow");
case 27: return("@S: Underflow");
case 28: return("@M: Overflow");
case 29: return("@M: Underflow");
case 30: return("@D: Overflow");
case 31: return("@D: Underflow");
case 32: return("@D: Divide fault");
case 33: return(".P,.M: Number of arguments do not match");
case 34: return("Illegal format syntax.");
case 35: return("FORMAT: Format parentheses nested too deep.\n\
$TEST:  Illegal array index or illegal DO parameter.");
case 36: return("No format specification for remaining data.");
case 37: return("Superfluous number in format ignored.");
case 38: return("Zero or negative count in format.");
case 39: return("Missing or negative d-field in format. Zero assumed.");
case 40: return("Illegal character in numeric input.");
case 41: return("I/O list item in wrong mode. Value used anyway.");
case 42: return("Input value underflow. Zero used.");
case 43: return("Input value overflow. Maximum used.");
case 44: return("Output exponent too large. Treated modulo 1000.");
case 50: return("Symbol table overflow.");
case 51: return("Overflow.");
case 52: return("Underflow.");
case 53: return("Integer constant is too large.");
case 54: return("Illegal character constant.");
case 55: return("Statement contains more than 1253 characters.");
case 56: return("Illegal continuation card.");
case 57: return("An identifier is assigned to more than one common block.");
case 58: return("A label is referenced but not defined.");
case 59: return("An equivalence chain extends storage in the wrong direction of a \n\
common block beyond the last assignment for that block made\n\
directly by a COMMON statement.");
case 60: return("Illegal syntax in a subscript or subscript expressions.");
case 61: return("Mode incompatibility of arithmetic operands.");
case 62: return("Illegal assign variable.");
case 63: return("Illegal assignment (mode incompatibility).");
case 64: return("Illegal arithmetic or logical expression.");
case 65: return("Missing right parenthesis.");
case 66: return("Illegal function reference.\n\
Unbalanced parentheses.\n\
Illegal array element reference.");
case 67: return("Internally defined variable name used as a function or array \n\
element name.");
case 68: return("Mode incompatibility in exponentiation operands.");
case 69: return("Illegal operand following a **.");
case 70: return("Illegal operand following a +, -, *, or /.");
case 71: return("Constant subscript too large.");
case 72: return("Illegal unit specification.\n\
Unbalanced parentheses.\n\
Illegal format specification\n\
Illegal I/O list\n\
Illegal or non-existant Logical Unit number.");
case 73: return("Illegal or non-existant Logical Unit number.");
case 74: return("Mode of identifier and mode of data item do not match.");
case 75: return("Number of list & data items do not match.\n\
Illegal delimiter or implied DO\n\
Dummy arg appears in DATA statement.");
case 76: return("Illegal member of a call list.");
case 77: return("Identifier used previously, not as an external procedure name.");
case 78: return("Illegal mode combination in a relational expression.");
case 79: return("Illegal subscript or parameter list.");
case 80: return("Illegal logical variable, constant or operator.\n\
Illegal arithmetic expression.");
case 81: return("Unrecognizable statement.");
case 82: return("Illegal syntax in an I/O list.\n\
Illegal implied DO\n\
Unbalanced parentheses.");
case 83: return("IF contains a DO statement or another IF.");
case 84: return("Multiply defined statement label.");
case 85: return("A label reference contains more than five characters or\n\
contains non-numeric characters.");
case 86: return("An identifier contains more than six characters.");
case 87: return("An array may not have more than three dimensions.");
case 88: return("A variable in the DO-list is not an integer.\n\
Missing equal sign\n\
Illegal delimiter.");
case 89: return("Unrecognizable statement.\n\
Illegal statement in a DO-loop\n\
Specification statement occurs after the first executable statement\n\
Return statement used in the main program.");
case 90: return("Illegal identifier or delimiter.\n\
An identifier may not be declared as an array because it has\n\
previously been used as a variable.");
case 91: return("Illegal identifier or delimiter.\n\
An entry has been previously defined.");
case 92: return("Unbalanced parentheses.\n\
Illegal identifier\n\
Less than two members in an equivalence list\n\
Two members of different common blocks are equivalent.");
case 93: return("Illegal number of slashes.\n\
Illegal member\n\
Member is adjustably dimensioned.");
case 94: return("No identifier.\n\
No array declarator\n\
Illegal identifier or delimiter\n\
An identifier may not be declared as an array because it has\n\
been used as a variable.");
case 95: return("Unbalanced parenthesis in a formal parameter list.");
case 96: return("Illegal formal parameter list.");
case 97: return("A main program function, sub-program, or subroutine does not\n\
contain an executable statement\n\
A SUBROUTINE or FUNCTION statement appears that is not the first\n\
program statement.");
case 98: return("Illegal statement.");
case 99: return("Missing END statement or specification statement out of order.");
case 100: return("Unrecognizable FORMAT statement.");
case 101: return("DO or statement label of DO termination previously defined.");
case 102: return("Illegal syntax in IMPLICIT statement.");
case 103: return("Illegal type declaration.");
case 104: return("Statement label used as a DO termination not found.");
case 105: return("Number of subscripts and dimensions does not agree.");
case 106: return("Syntax error in character constant.");
case 107: return("Syntax error in address constant.");
case 108: return("Address constant or variable used in real, double precision,\n\
complex or logical expression.");
case 109: return("Address constant or variable appears in exponentiation.");
case 110: return("Syntax error in hexadecimal constant.");
case 111: return("Function variable not used as an assign variable within FUNCTION.");
case 112: return("Trace option specified in BLOCK DATA subprogram.");
case 113: return("Illegal OPTION statement.");
case 114: return("Assign variable is not ADDRESS or INTEGER*4 type.");
case 115: return("Address variable appears in a DATA statement.");
case 116: return("Item in blank COMMON used in DATA statement.");
case 117: return("Mixed or invalid modes in argument list.");
default: return("Unknown error number.");
}
}
