/*
    Copyright 2015, Darren Smith

    This file is part of c_dev_utils.

    c_dev_utils is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    c_dev_utils is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with c_dev_utils.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OUT( X ) \
  printf("%s, %d, %s\n", #X, X, strerror( X ) )

int main(int argc , const char** argv)
{

#include "cerror_generated.c"

  return 0;
}
