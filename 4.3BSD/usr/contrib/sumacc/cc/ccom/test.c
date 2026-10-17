test()
 { 
   short test2();
   return( test2(1) >= test2(2) );
 }

struct s { short a, b; } *p;

short test2(a, b, c)
 short a,b,c;
  {
    if ( p->b == b)
    return( a*b + a*c );
  }

