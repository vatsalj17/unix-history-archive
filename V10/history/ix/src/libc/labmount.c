labmount(fd, cp)
void *cp;
{
	return syscall(64+42, fd, cp);
}
