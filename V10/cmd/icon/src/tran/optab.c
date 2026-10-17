#include "lex.h"

/*
 * State tables for operator recognition.
 */

struct optab state0[] = {       /* initial state */
   { ',', A_Immret, (char *) &toktab[ 59] },      /* ","     */
   { '.', A_Immret, (char *) &toktab[ 63] },      /* "."     */
   { '[', A_Immret, (char *) &toktab[ 70] },      /* "["     */
   { ']', A_Immret, (char *) &toktab[ 99] },      /* "]"     */
   { '(', A_Immret, (char *) &toktab[ 79] },      /* "("     */
   { ')', A_Immret, (char *) &toktab[100] },      /* ")"     */
   { ';', A_Immret, (char *) &toktab[101] },      /* ";"     */
   { '{', A_Immret, (char *) &toktab[ 69] },      /* "{"     */
   { '}', A_Immret, (char *) &toktab[ 98] },      /* "}"     */
   { '!', A_Immret, (char *) &toktab[ 54] },      /* "!"     */
   { '\\', A_Immret, (char *) &toktab[ 53] },      /* "\\"    */
   { ':', A_Goto,   (char *) state1       },      /* ":" ... */
   { '<', A_Goto,   (char *) state2       },      /* "<" ... */
   { '>', A_Goto,   (char *) state4       },      /* ">" ... */
   { '=', A_Goto,   (char *) state5       },      /* "=" ... */
   { '|', A_Goto,   (char *) state3       },      /* "|" ... */
   { '+', A_Goto,   (char *) state7       },      /* "+" ... */
   { '-', A_Goto,   (char *) state8       },      /* "-" ... */
   { '*', A_Goto,   (char *) state9       },      /* "*" ... */
   { '^', A_Goto,   (char *) state6       },      /* "^" ... */
   { '~', A_Goto,   (char *) state29      },      /* "~" ... */
   { '/', A_Goto,   (char *) state21      },      /* "/" ... */
   { '%', A_Goto,   (char *) state30      },      /* "%" ... */
   { '?', A_Goto,   (char *) state36      },      /* "?" ... */
   { '&', A_Goto,   (char *) state38      },      /* "&" ... */
   { '@', A_Goto,   (char *) state40      },      /* "@" ... */
   { 0,   A_Error,  0            }
   };

struct optab state1[] = {       /* ":" */
   { '=', A_Goto,   (char *) state10      },      /* ":=" ... */
   { 0,   A_Return, (char *) &toktab[ 58] }       /* ":"      */
   };

struct optab state2[] = {       /* "<" */
   { '-', A_Goto,   (char *) state11      },      /* "<-" ... */
   { '<', A_Goto,   (char *) state32      },      /* "<<" ... */
   { ':', A_Goto,   (char *) state46      },      /* "<:" ... */
   { '=', A_Goto,   (char *) state56      },      /* "<=" ... */
   { 0,   A_Return, (char *) &toktab[ 90] }       /* "<"      */
   };

struct optab state3[] = {       /* "|" */
   { '|', A_Goto,   (char *) state22      },      /* "||" ... */
   { 0,   A_Return, (char *) &toktab[ 55] }       /* "|"      */
   };

struct optab state4[] = {       /* ">" */
   { '>', A_Goto,   (char *) state33      },      /* ">>" ... */
   { ':', A_Goto,   (char *) state44      },      /* ">:" ... */
   { '=', A_Goto,   (char *) state57      },      /* ">=" ... */
   { 0,   A_Return, (char *) &toktab[ 88] }       /* ">"      */
   };

struct optab state5[] = {        /* "=" */
   { '=', A_Goto,   (char *) state12      },      /* "==" ... */
   { ':', A_Goto,   (char *) state42      },      /* "=:" ... */
   { 0,   A_Return, (char *) &toktab[ 86] }       /* "="      */
   };

struct optab state6[] = {        /* "^" */
   { ':', A_Goto,   (char *) state23      },      /* "^:" ... */
   { 0,   A_Return, (char *) &toktab[ 56] }       /* "^"      */
   };

struct optab state7[] = {       /* "+" */
   { ':', A_Goto,   (char *) state15      },      /* "+:" ... */
   { '+', A_Goto,   (char *) state16      },      /* "++" ... */
   { 0,   A_Return, (char *) &toktab[ 93] }       /* "+"      */
   };

struct optab state8[] = {        /* "-" */
   { ':', A_Goto,   (char *) state17      },      /* "-:" ... */
   { '-', A_Goto,   (char *) state18      },      /* "--" ... */
   { 0,   A_Return, (char *) &toktab[ 81] }       /* "-"      */
   };

struct optab state9[] = {        /* "*" */
   { ':', A_Goto,   (char *) state19      },      /* "*:" ... */
   { '*', A_Goto,   (char *) state20      },      /* "**" ... */
   { 0,   A_Return, (char *) &toktab[105] }       /* "*"      */
   };

struct optab state10[] = {       /* ":=" */
   { ':', A_Immret, (char *) &toktab[107] },      /* ":=:" */
   { 0,   A_Return, (char *) &toktab[ 35] }       /* ":="  */
   };

struct optab state11[] = {       /* "<-" */
   { '>', A_Immret, (char *) &toktab[ 97] },      /* "<->" */
   { 0,   A_Return, (char *) &toktab[ 96] }       /* "<-"  */
   };

struct optab state12[] = {       /* "==" */
   { '=', A_Goto,   (char *) state61      },      /* "===" ... */
   { ':', A_Goto,   (char *) state48      },      /* "==:" ... */
   { 0,   A_Return, (char *) &toktab[ 73] }       /* "=="  */
   };

struct optab state13[] = {       /* "~=" */
   { '=', A_Goto,   (char *) state14      },      /* "~==" ... */
   { ':', A_Goto,   (char *) state43      },      /* "~=:" ... */
   { 0,   A_Return, (char *) &toktab[ 91] }       /* "~="      */
   };

struct optab state14[] = {       /* "~==" */
   { ':', A_Goto,   (char *) state49      },      /* "~==:" ... */
   { '=', A_Goto,   (char *) state60      },      /* "~===" ... */
   { 0,   A_Return, (char *) &toktab[ 78] }       /* "~=="  */
   };

struct optab state15[] = {       /* "+:" */
   { '=', A_Immret, (char *) &toktab[ 94] },      /* "+:=" */
   { 0,   A_Return, (char *) &toktab[ 92] }       /* "+:"  */
   };

struct optab state16[] = {       /* "++" */
   { ':', A_Goto,   (char *) state24      },      /* "++:" ... */
   { 0,   A_Return, (char *) &toktab[109] }       /* "++"      */
   };

struct optab state17[] = {       /* "-:" */
   { '=', A_Immret, (char *) &toktab[ 82] },      /* "-:=" */
   { 0,   A_Return, (char *) &toktab[ 80] }       /* "-:"  */
   };

struct optab state18[] = {       /* "--" */
   { ':', A_Goto,   (char *) state25      },      /* "--:" ... */
   { 0,   A_Return, (char *) &toktab[ 64] }       /* "--" */
   };

struct optab state19[] = {      /* "*:" */
   { '=', A_Immret, (char *) &toktab[106] },      /* "*:=" */
   { 0,   A_Error,  0            }
   };

struct optab state20[] = {       /* "**" */
   { ':', A_Goto,   (char *) state26      },      /* "**:" ... */
   { 0,   A_Return, (char *) &toktab[ 67] }       /* "**"      */
   };

struct optab state21[] = {       /* "/" */
   { ':', A_Goto,   (char *) state27      },      /* "/:" ... */
   { 0,   A_Return, (char *) &toktab[103] }       /* "/"      */
   };

struct optab state22[] = {       /* "||" */
   { ':', A_Goto,   (char *) state28      },      /* "||:" ... */
   { '|', A_Goto,   (char *) state34      },      /* "|||" ... */
   { 0,   A_Return, (char *) &toktab[ 60] }       /* "||"      */
   };

struct optab state23[] = {       /* "^:" */
   { '=', A_Immret, (char *) &toktab[ 57] },      /* "^:=" */
   { 0,   A_Error,  0            }
   };

struct optab state24[] = {       /* "++:" */
   { '=', A_Immret, (char *) &toktab[110] },      /* "++:=" */
   { 0,   A_Error,  0            }
   };

struct optab state25[] = {       /* "--:" */
   { '=', A_Immret, (char *) &toktab[ 65] },      /* "--:=" */
   { 0,   A_Error,  0            }
   };

struct optab state26[] = {       /* "**:" */
   { '=', A_Immret, (char *) &toktab[ 68] },      /* "**:=" */
   { 0,   A_Error,  0            }
   };

struct optab state27[] = {       /* "/:" */
   { '=', A_Immret, (char *) &toktab[104] },      /* "/:=" */
   { 0,   A_Error,  0            }
   };

struct optab state28[] = {      /* "||:" */
   { '=', A_Immret, (char *) &toktab[ 61] },      /* "||:=" */
   { 0,   A_Error,  0            }
   };

struct optab state29[] = {       /* "~" */
   { '=', A_Goto,   (char *) state13      },      /* "~=" ... */
   { 0,   A_Return, (char *) &toktab[108] }       /* "~"      */
   };

struct optab state30[] = {       /* "%" */
   { ':', A_Goto,   (char *) state31      },      /* "%:" ... */
   { 0,   A_Return, (char *) &toktab[ 83] }       /* "%"      */
   };

struct optab state31[] = {       /* "%:" */
   { '=', A_Immret, (char *) &toktab[ 84] },      /* "%:=" */
   { 0,   A_Error,  0            }
   };

struct optab state32[] = {       /* "<<" */
   { ':', A_Goto,   (char *) state52      },      /* "<<:" ... */
   { '=', A_Goto,   (char *) state58      },      /* "<<=" ... */
   { 0,   A_Return, (char *) &toktab[ 77] }       /* "<<"     */
   };

struct optab state33[] = {       /* ">>" */
   { ':', A_Goto,   (char *) state50      },      /* ">>:" ... */
   { '=', A_Goto,   (char *) state59      },      /* ">>=" ... */
   { 0,   A_Return, (char *) &toktab[ 75] }       /* ">>"     */
   };

struct optab state34[] = {       /* "|||" */
   { ':', A_Goto,   (char *) state35      },      /* "|||:" ... */
   { 0,   A_Return, (char *) &toktab[ 71] }       /* "|||"      */
   };

struct optab state35[] = {       /* "|||:" */
   { '=', A_Immret, (char *) &toktab[ 72] },      /* "|||:=" */
   { 0,   A_Error,  0            }
   };

struct optab state36[] = {        /* "?" */
   { ':', A_Goto,   (char *) state37      },      /* "?:" ... */
   { 0,   A_Return, (char *) &toktab[ 95] }       /* "?"      */
   };

struct optab state37[] = {       /* "?:" */
   { '=', A_Immret, (char *) &toktab[102] },      /* "?:=" */
   { 0,   A_Error,  0            }
   };

struct optab state38[] = {        /* "&" */
   { ':', A_Goto,   (char *) state39      },      /* "&:" ... */
   { 0,   A_Return, (char *) &toktab[ 62] }       /* "&"      */
   };

struct optab state39[] = {       /* "&:" */
   { '=', A_Immret, (char *) &toktab[ 38] },      /* "&:=" */
   { 0,   A_Error,  0            }
   };

struct optab state40[] = {        /* "@" */
   { ':', A_Goto,   (char *) state41      },      /* "@:" ... */
   { 0,   A_Return, (char *) &toktab[ 36] }       /* "@"      */
   };

struct optab state41[] = {      /* "@:" */
   { '=', A_Immret, (char *) &toktab[ 37] },      /* "@:=" */
   { 0,   A_Error,  0            }
   };

struct optab state42[] = {       /* "=:" */
   { '=', A_Immret, (char *) &toktab[ 39] },      /* "=:=" */
   { 0,   A_Error,  0            }
   };

struct optab state43[] = {       /* "~=:" */
   { '=', A_Immret, (char *) &toktab[ 45] },      /* "~=:=" */
   { 0,   A_Error,  0            }
   };

struct optab state44[] = {       /* ">:" */
   { '=', A_Immret, (char *) &toktab[ 42] },      /* ">:=" */
   { 0,   A_Error,  0            }
   };

struct optab state45[] = {       /* ">=:" */
   { '=', A_Immret, (char *) &toktab[ 41] },      /* ">=:=" */
   { 0,   A_Error,  0            }
   };

struct optab state46[] = {      /* "<:" */
   { '=', A_Immret, (char *) &toktab[ 44] },      /* "<:=" */
   { 0,   A_Error,  0            }
   };

struct optab state47[] = {       /* "<=:" */
   { '=', A_Immret, (char *) &toktab[ 43] },      /* "<=:=" */
   { 0,   A_Error,  0            }
   };

struct optab state48[] = {       /* "==:" */
   { '=', A_Immret, (char *) &toktab[ 47] },      /* "==:=" */
   { 0,   A_Error,  0            }
   };

struct optab state49[] = {       /* "~==:" */
   { '=', A_Immret, (char *) &toktab[ 52] },      /* "~==:=" */
   { 0,   A_Error,  0            }
   };

struct optab state50[] = {      /* ">>:" */
   { '=', A_Immret, (char *) &toktab[ 49] },      /* ">>:=" */
   { 0,   A_Error,  0            }
   };

struct optab state51[] = {       /* ">>=:" */
   { '=', A_Immret, (char *) &toktab[ 48] },      /* ">>=:=" */
   { 0,   A_Error,  0            }
   };

struct optab state52[] = {       /* "<<:" */
   { '=', A_Immret, (char *) &toktab[ 51] },      /* "<<:=" */
   { 0,   A_Error,  0            }
   };

struct optab state53[] = {       /* "<<=:" */
   { '=', A_Immret, (char *) &toktab[ 50] },      /* "<<=:=" */
   { 0,   A_Error,  0            }
   };

struct optab state54[] = {      /* "===:" */
   { '=', A_Immret, (char *) &toktab[ 40] },      /* "===:=" */
   { 0,   A_Error,  0            }
   };

struct optab state55[] = {       /* "~===:" */
   { '=', A_Immret, (char *) &toktab[ 46] },      /* "~===:=" */
   { 0,   A_Error,  0            }
   };

struct optab state56[] = {        /* "<=" */
   { ':', A_Goto,   (char *) state47      },      /* "<=:" ... */
   { 0,   A_Return, (char *) &toktab[ 89] }       /* "<="      */
   };

struct optab state57[] = {        /* ">=" */
   { ':', A_Goto,   (char *) state45      },      /* ">=:" ... */
   { 0,   A_Return, (char *) &toktab[ 87] }       /* ">="      */
   };

struct optab state58[] = {        /* "<<=" */
   { ':', A_Goto,   (char *) state53      },      /* "<<=:" ... */
   { 0,   A_Return, (char *) &toktab[ 76] }       /* "<<="      */
   };

struct optab state59[] = {       /* ">>=" */
   { ':', A_Goto,   (char *) state51     },      /* ">>=:" ... */
   { 0,   A_Return, (char *) &toktab[ 74] }       /* ">>="      */
   };

struct optab state60[] = {        /* "~===" */
   { ':', A_Goto,   (char *) state55      },      /* "~===:" ... */
   { 0,   A_Return, (char *) &toktab[ 85] }       /* "~==="      */
   };

struct optab state61[] = {        /* "===" */
   { ':', A_Goto,   (char *) state54      },      /* "===:" ... */
   { 0,   A_Return, (char *) &toktab[ 66] }       /* "==="      */
   };
