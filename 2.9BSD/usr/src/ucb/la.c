/*
 * la: print the load average.
 */
main()
{
	double vec[3];

	if (loadav(vec) < 0) {
		perror("loadav: gldav");
		exit(1);
	}
	printf("load %.02f %.02f %.02f\n", vec[0], vec[1], vec[2]);
	exit(0);
}
