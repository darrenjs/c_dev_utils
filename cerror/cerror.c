#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#define OUT( X ) \
  printf(#X " %d \n", X )

int main(int argc , const char** argv)
{

#ifdef ECONNREFUSED
  OUT( ECONNREFUSED );
#endif

#ifdef E2BIG
  OUT( E2BIG );
#endif

#ifdef EACCES
  OUT( EACCES );
#endif

#ifdef EADDRINUSE
  OUT( EADDRINUSE );
#endif

#ifdef EADDRNOTAVAIL
  OUT( EADDRNOTAVAIL );
#endif

#ifdef EAFNOSUPPORT
  OUT(EAFNOSUPPORT  );
#endif

  return 0;
}
