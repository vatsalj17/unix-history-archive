/* amp.h - global defs (not many) */

/* Structure of translation table */

struct hstab
{
    char   *name;                 /* Host (NODE) Name to look for       */
    char   *at;                   /* use this a hostname                */
    char   *dot;                  /* append this to mbox portion        */
};

/*  addresses of the form "<mbox> @ name" become "<mbox>.dot @ at"      */

long amp_hdr();
