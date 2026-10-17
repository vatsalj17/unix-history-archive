/*----LENSTR----Calculate length of a string as the number
  of characters before a null byte or the total length of the
  string less any blanks on the right end.
  PLWard, USGS, Menlo Park 2/19/80
*/

int lenstr_(string,length) char string[]; long length;
{
     int i;
     for(i=0; i<length && string[i] != '\0'; i++);
     if (i == length) for( ; i > 0 && string[i-1] == ' ';i--);
     return(i);
}
