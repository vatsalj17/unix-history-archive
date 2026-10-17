long syscall();		/* fraud */
long tell(fd)
{
	return syscall(64+38, fd);
}
