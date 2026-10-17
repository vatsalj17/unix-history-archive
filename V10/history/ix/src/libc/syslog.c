syslog(a1, a2, a3)
{
	return syscall(64+15, a1, a2, a3);
}
