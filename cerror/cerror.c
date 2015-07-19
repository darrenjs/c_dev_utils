#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define OUT( X ) \
  printf(#X " %d \n", X )

int main(int argc , const char** argv)
{

#include "cerror_generated.c"

  return 0;
}
