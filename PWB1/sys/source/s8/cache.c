#define THRESH	210
main(){
	struct {
		long user;
		long sys;
		long childuser;
		long childsys;
	} buffer;
	int i, j;

	for(i=1;i<=150;++i) {
		for(j=0;j<3000;++j);
		write(1,"",0);
	}
	times(&buffer);
printf("%D: ", buffer.user);
	if(buffer.user > THRESH) {
		printf("Cache buffer may not be working!\n");
		exit(1);
	} else {
		printf("Cache buffer seems OK.\n");
		exit(0);
	}
}
