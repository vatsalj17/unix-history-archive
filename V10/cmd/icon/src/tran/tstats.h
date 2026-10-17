#ifdef TranStats
#define FirstTok CSETLIT
#define LastYacc UNIONASGN
#define REPALT LastYacc+1
#define CCLS LastYacc+2
#define EMPTY LastYacc+3
#define INVOKE LastYacc+4
#define KEYWORD LastYacc+5
#define LIST LastYacc+6
#define SECTION LastYacc+7
#define COMPOUND LastYacc+8
#define UDOT LastYacc+9
#define UBACK LastYacc+10
#define UCARET LastYacc+11
#define UPLUS LastYacc+12
#define UEQUAL LastYacc+13
#define UMINUS LastYacc+14
#define USTAR LastYacc+15
#define UQUES LastYacc+16
#define USLASH LastYacc+17
#define LastTok USLASH
#define TokSize LastTok-FirstTok

/* extern int TokCount[]; */

#define TokInc(x) TokCount[x-FirstTok]++
#define TokDec(x) TokCount[x-FirstTok]--
#else TranStats
#define TokInc(x)
#define TokDec(x)
#endif TranStats
