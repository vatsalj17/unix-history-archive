getgroup (realid, effecid)         /* group info for V7 Unix             */
    int   *realid,
	*effecid;
{
    *realid = getgid ();

    *effecid = getegid();
}
