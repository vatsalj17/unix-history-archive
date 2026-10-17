#include	<stdio.h>
#include 	<a.out.h>
/*
	size -- determine object size
*/
int a_magic[] = {A_MAGIC1,A_MAGIC2,A_MAGIC3,A_MAGIC4, 0430, 0431, 0};
main(argc, argv)
char **argv;
{
	struct exec buf;
	long sum, coresize;
	int gorp,i;
	FILE *f;
	if (argc==1) {
		*argv = "a.out";
		argc++;
		--argv;
	}
	gorp = argc;
	while(--argc) {
		++argv;
		if ((f = fopen(*argv, "r"))==NULL) {
			printf("size: %s not found\n", *argv);
			continue;
		}
		fread((char *)&buf, sizeof(buf), 1, f);
		for(i=0;a_magic[i];i++)
			if(a_magic[i] == buf.a_magic) break;
		if(a_magic[i] == 0) {
			printf("size: %s not an object file\n", *argv);
			fclose(f);
			continue;
		}
		if (gorp>2)
			printf("%s: ", *argv);
		printf("%u+", buf.a_text);
/* wnj added */
		coresize = buf.a_text;
		if (buf.a_magic == 0430 || buf.a_magic == 0431) {
			unsigned sizes[8];
			fread(sizes, sizeof sizes, 1, f);
			printf("(");
			for (i = 1; i < 8; i++)
				if (sizes[i]) {
					coresize += sizes[i];
					if (i > 1)
						printf(",");
					printf("%u", sizes[i]);
				}
			printf(")+");
		}
/* end wnj added */
		printf("%u+%u = ", buf.a_data,buf.a_bss);
		sum = (long) buf.a_text + (long) buf.a_data + (long) buf.a_bss;
		printf("%Db = 0%Ob", sum, sum);
		if (coresize != buf.a_text)
			printf(" (%D total text)", coresize);
		printf("\n");
		fclose(f);
	}
}
