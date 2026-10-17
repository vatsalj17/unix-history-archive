#include <string.h>

strvec(str, vec, sep)
register char	*str, **vec, *sep;
{
	register int	i;

	for (i = 0; vec[i] = strtok(i ? 0 : str, sep); i++);
	return i;
}
