#include <stdio.h>

int head[8];
int ov[8];
int buf[8*1024];

int t, tc, i;

main(argc, argv)
char *argv[];
{
	int n;

	if (argc > 1)
		n = atoi(argv[1]);
	else
		n = 1;
	read(0, (char *)&head, sizeof head);
	head[0] = 0410;
	t = head[1];
	read(0, (char *)&ov, sizeof ov);
	head[1] = (head[1] + 017777) & ~017777;
	head[1] += ov[n];
	write(1, (char *)&head, sizeof head);
	read(0, buf, t);
	write(1, buf, t);
	t = (t + 017777) & ~ 017777;
	lseek(1, (long)(t+020), 0);
	for (i = 1; i < 8; i++) {
		read(0, buf, ov[i]);
		if (i == n)
			write(1, buf, ov[i]);
	}
	while((t = read(0, buf, 16*1024)) > 0)
		write(1, buf, t);
}
