getwho (realid, effecid)          /* user info for V7 Unix              */
    int   *realid,
	*effecid;
{
    *realid = getuid ();

    *effecid = geteuid();
}
