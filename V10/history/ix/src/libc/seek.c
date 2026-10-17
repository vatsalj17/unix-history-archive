seek(fd, offset, how)
long offset;
{
	return syscall(64+37, fd, offset, how);
}
