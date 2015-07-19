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
#include <time.h>


void display_row(const char * label, time_t t)
{
  char ctime_str[256];
  memset(ctime_str, 0, sizeof(ctime_str) );
  char* r = ctime_r(&t, ctime_str);
  ctime_str[ sizeof(ctime_str)-1 ] = '\0';

  if (r)
    printf("%-20s: % 20ld : %s", label, t, ctime_str);
  else
    printf("%-20s: % 20ld : ctime_r failed errno %d\n", label, t, errno);

}


/* Calculate the time as it was at the start of today */
time_t time_at_start_of_day()
{
  tzset(); // required, because we are using localtime_r variant

  struct tm     timedesc;
  time_t timenow = time(NULL);

  localtime_r(&timenow, &timedesc);

  // reset the timedese
  timedesc.tm_sec  = 0;
  timedesc.tm_min  = 0;
  timedesc.tm_hour = 0;

  // now build a time_t for start of day
  const time_t sod = mktime( &timedesc );

  return sod;
}


int main(int argc, const char** argv)
{
  display_row("now", time(NULL));
  display_row("start of day", time_at_start_of_day());

  int i;
  for (i = 1; i < argc; ++i)
  {
    display_row(argv[i], atol( argv[i] ));
  }

  if ( argc == 1 )
  {
    printf("(you can provide time_t args on the command line)\n");
  }

  return 0;
}

