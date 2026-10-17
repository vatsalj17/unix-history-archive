/* Fortran string concatenation PLWard 3/20/80 */
char string_(strout,outlen,numargs,params)
     long outlen;
     int *numargs;
     char strout[],params[];
{
     int i,j,k=0;
     char **arg;
     long *lengths;

     arg = &params;
     lengths = &arg[*numargs];

     for(i=0; i < *numargs ; i++){
          for( j=0; j < lengths[i]; j++,k++)
               if(k < outlen) strout[k] = arg[i][j];
          if (lengths[i] > 1)
               while (strout[k-1] == ' ' || strout[k-1] == '\0')
                    if ( k > 0 ) k--;
     }
     strout[k] = '\0';
}
