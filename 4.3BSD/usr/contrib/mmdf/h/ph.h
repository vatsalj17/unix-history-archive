struct ps_rstruct {             /* save last reply obtained           */
    int     sm_rval;            /* rp.h value for reply               */
    int     sm_rlen;            /* current lengh of reply string      */
    char    sm_rgot;            /* true, if have a reply              */
    char    sm_rstr[LINESIZE];  /* human-oriented reply text          */
};

#define rp_structlen(bufnam) (strlen (bufnam.rp_line) + sizeof (bufnam.rp_val))
