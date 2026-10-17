struct buf {
	int fildes;
	int left;
	char *next;
	char buff[512];};

struct buf *abps[15];	/*array of buffer pointers*/

copen(filename,type)
  char *filename;
  int type;
{
	register struct buf *bp;
	register int fd;

	if((bp = alloc(sizeof(*bp))) < 0) return(-1);

	if((fd=bp->fildes=open(filename,0)) < 0 || fd > 14)
			return(-1);
	abps[fd] = bp;	/* save buffer pointer addr */
	bp->left = 0;

	return(fd);
}

cgetc(fd)
  int fd;
{

	register struct buf *bp;
	register char c;

	bp = abps[fd];
	if(--bp->left < 0) {
		bp->left = c = read(bp->fildes,bp->buff,sizeof(bp->buff))-1;
		if(c<0) return('\0');
		bp->next = bp->buff;
			       }
	return(*bp->next++);

}

cclose(fd)
  int fd;
{
	free(abps[fd]);
	close(fd);
	abps[fd]=0;
}

cexit(rc)
  int rc;
{
	register int fd;

	for(fd=3;fd<15;fd++)
		if (abps[fd] != 0) cclose(fd);
	exit(rc);
}

