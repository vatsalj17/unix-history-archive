/*  environment for address parser */

#define AP_SAME   0000          /* do not transorm the address */
#define AP_733    0001          /* follow RFC #733 rules */
#define AP_822    0002          /* follow RFC #822 rules */
#define AP_NODOTS 0004          /* strip down to hostname on next hop */
#define AP_BIG    0010          /* Use Big-endian domains, FLAG */

struct ap_node
{
    char    ap_obtype,            /* parsing type of this object        */
#define APV_NIL  0
#define APV_NAME 1                /* personal name                      */
#define APV_MBOX 2                /* mailbox-part of address            */
#define APV_DOMN 3                /* host-part of address               */
#define APV_DTYP 4                /* "data-type" (e.g., :include:...,)  */
#define APV_CMNT 5                /* comment (...)                      */
#define APV_WORD 6                /* generic word                       */
#define APV_PRSN 7                /* start of personal addr list <...>  */
#define APV_NPER 8                /* name of person                     */
#define APV_EPER 9                /* end of personal address list       */
#define APV_GRUP 10               /* start of group address list x:..;  */
#define APV_NGRP 11               /* name of group                      */
#define APV_EGRP 12               /* end of group list                  */
#define APV_DLIT 13               /* domain literal                     */
	    ap_ptrtype;           /* next node is continuation of this  */
				  /*   address, start of new, or null   */
#define APP_NIL  0                /* there is no next node              */
#define APP_ETC  1                /* next is part of this address       */
#define APP_NXT  2                /* next is start of new address       */

    char   *ap_obvalue;           /* pointer to string value of object  */
    struct ap_node  *ap_chain;    /* pointer to next node               */
};

typedef struct ap_node *AP_ptr;

struct ap_prevstruct
{                                 /* for use when getting indirect input*/
    FILE *ap_curfp;               /* handle on current file input       */
    struct ap_prevstruct   *ap_prvptr;
				  /* next input down the stack, using...*/
    int     (*ap_prvgfunc) ();    /* getchar function for that input    */
    int     ap_opeek,             /* with this as peek-ahead char for it*/
	    ap_ogroup,            /* nesting level of group list        */
	    ap_opersn;            /* nesting level of personal list     */
};


extern  char	ap_llex;
extern  AP_ptr  ap_pstrt;
extern  AP_ptr  ap_pcur;
extern	int	ap_1adr();
extern	AP_ptr	ap_1delete();
extern	AP_ptr	ap_add();
extern	AP_ptr	ap_alloc();
extern	AP_ptr	ap_append();
extern	int	ap_char();
extern	char *	ap_dmflip();
extern	int	ap_dmnormalize();
extern	int	ap_flget();
extern	int	ap_fpush();
extern	int	ap_lex();
extern	AP_ptr	ap_move();
extern	AP_ptr	ap_new();
extern	AP_ptr	ap_normalize();
extern	char *	ap_p2s();
extern	AP_ptr	ap_pinit();
extern	int	ap_ppush();
extern	char *	ap_s2p();
extern	AP_ptr	ap_s2tree();
extern	AP_ptr	ap_sqdelete();
extern	AP_ptr	ap_sqinsert();
extern	AP_ptr	ap_sqmove();
extern	AP_ptr	ap_t2parts();
extern	AP_ptr	ap_t2s();
