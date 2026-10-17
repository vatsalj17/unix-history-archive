/*
 * nochk - if flag == 0 surrender T_NOCHK rights on fd
 *		   != 0 regain rightful T_NOCHK rights
 * return values:
 *	0		had surrendered rights
 *	T_NOCHK		had not
 *	-1		EBADF - file not open
 */
nochk(fd, flag) {
	return syscall(64+29, fd, flag);
}
