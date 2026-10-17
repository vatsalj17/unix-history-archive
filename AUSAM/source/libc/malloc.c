	/*
	 *	new malloc returns 0 on failure not -1
	 *
	 *	primarily for new stdio library from pwb
	 */
malloc(size)
{
	register p;

	p = alloc(size);
	return p == -1 ? 0 : p;
}
