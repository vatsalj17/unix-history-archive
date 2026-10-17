/* ap_norm.h - table declaration for ap_normalize.c */

/* Structure of translation table */

struct ap_hstab
{
    char   *name;                 /* Host (NODE) Name to look for       */
    char   *at;                   /* use this a hostname                */
    char   *dot;                  /* append this to mbox portion        */
};
