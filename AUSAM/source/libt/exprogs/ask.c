extern double atof();

ask( posx, posy, prompt, answer )
int	posx,	posy;
char *prompt,	*answer;
{
	char map;

	alpha();
	sxputsa( posx, posy, prompt );
	map = 'a'-'A';
	while( ( *answer = getchar() ) != '\n' ){
		if( *answer <= 'Z' ) if( *answer >= 'A' ) *answer++ =+ map;
		else answer++;
	}
	*answer = '\0';
}
