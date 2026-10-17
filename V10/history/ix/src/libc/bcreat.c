bcreat(filename, mode, udbuf)
char *filename, *udbuf;
{
	return syscall(64+14, filename, mode, udbuf);
}
