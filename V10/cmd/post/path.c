/*
 * Generate full pathname
 */

char *
path(file)
char	*file;
{
	static char	root[256];
	char	*getenv(), *post = getenv("POST");

	if (*file == '/' || *file == '.')
		return file;
	sprintf(root, "%s/%s", post ? post : POST, file);
	return root;
}
