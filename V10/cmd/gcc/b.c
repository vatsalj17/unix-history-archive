bcopy(b1, b2, length)
	char *b1, *b2;
{
	memcpy(b2, b1, length);
}

bcmp(b1, b2, length)
	char *b1, *b2;
{
	return(memcmp(b1, b2, length));
}

bzero(b, length)
	char *b;
{
	memset(b, 0, length);
}
