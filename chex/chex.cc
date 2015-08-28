
#include "color.h"

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>


enum chex_theme
{
  THEME_RESET     = 0,
  THEME_HEX,
  THEME_PADOCTECT,
  THEME_PADBYTE
};

static char theme_colors[][COLOR_MAXLEN] = {
	GIT_COLOR_RESET,
	GIT_COLOR_NORMAL,	/* THEME_HEX */
	GIT_COLOR_RED,		/* THEME_PADOCTECT */
	GIT_COLOR_RED,		/* THEME_PADBYTE */
	GIT_COLOR_RED,		/* OLD */
	GIT_COLOR_GREEN,	/* NEW */
	GIT_COLOR_YELLOW,	/* COMMIT */
	GIT_COLOR_BG_RED,	/* WHITESPACE */
	GIT_COLOR_NORMAL,	/* FUNCINFO */
};

const char* use_color(bool wantcolor, int ix)
{
  if (wantcolor)
    return theme_colors[ix];
  else
    return GIT_COLOR_RESET;
}


struct program_options
{
  enum pad_side { PADLEFT=0, PADRIGHT} padside;
};

/*


TODO:

* command lines arguments

* option to control padding on left or right

* option to space the hex output

* support strings

* arg to support what types to decode as

*/

#define PADBYTES 32

struct byte_array
{
  void*  buf;
  size_t buflen;

  // region of user data
  unsigned char*  ptr;
  size_t len;

  unsigned char* pad;
} bytes;


bool byte_array_ptr_in_user_region(const byte_array* ba, const unsigned char * p)
{
  return (p >= ba->ptr) && (p < ba->ptr+ba->len);
}


byte_array reserve(size_t len)
{
  byte_array ba;
  memset(&ba, 0, sizeof(byte_array));

  ba.buflen = len + 2 * PADBYTES;
  ba.buf    = malloc( ba.buflen );
  memset(ba.buf, 0, ba.buflen);

  ba.ptr = (unsigned char*) (ba.buf) + PADBYTES;
  ba.len = len;

  return ba;
}


char tohex(unsigned int i)
{
  static const char * alphabet="0123456789ABCDEF";
  return alphabet[ i & 0xF];
}



#define as_TYPE( T, FMT, SHOWCHAR )                                           \
  {                                                                  \
    printf("%s (%lu bytes)\n", #T, sizeof( T ));                      \
    T value = 0;                                                     \
    size_t width = sizeof( T );                                      \
    int pad = (width - ba.len % width) % width;                      \
    unsigned char* src = ba.ptr - pad;                               \
    /* src = ba.ptr; */                                              \
    int values = (ba.len + pad) / width;                             \
    for (int j = 0; j< values; ++j)                                  \
    {                                                                \
      memcpy(&value, src, width);                                    \
      /* display hex */                                              \
      for (size_t i = 0; i < width; ++i)                                \
      {                                                              \
        bool b = !byte_array_ptr_in_user_region(&ba,src+i);               \
        printf("%s%02X%s", use_color(1 && b, THEME_PADBYTE), *(src+i), use_color(1 && b, THEME_RESET)) ; \
      }                                                              \
      printf(" : " FMT , value);                                     \
      if (SHOWCHAR && isprint( value )) printf(" '%c'", (char)value);   \
      printf("\n");                                 \
      src += width;                                                  \
    }                                                                \
  }

void as_int(byte_array ba)
{
  printf("\n");
  as_TYPE( short, "%hi", false);
  printf("\n");
  as_TYPE( int, "%i", false );
  printf("\n");
  as_TYPE( long, "%li",false );
  printf("\n");
  as_TYPE( float, "%F", false);
  printf("\n");
  as_TYPE( double, "%F", false);
  printf("\n");
  as_TYPE( unsigned char, "%3hhu", true);
}


void dump(byte_array ba)
{
  for (size_t i = 0; i < ba.len; i++)
  {
    printf("%u\n", *(ba.ptr+i));
  }
  printf("\n");
}

byte_array hex_to_bytes(const char* s)
{
  byte_array ba = reserve( strlen(s) + 2);  // possible padding

  size_t found = 0;
  unsigned char* p = ba.ptr + 1;

  while (*s)
  {
    char c = tolower(*s);
    if (isdigit(c))
    {
      *p = *s - '0';
      found++;
      p++;
    }
    else if (c>='a' && c <= 'f')
    {
      *p = 10+(c-'a');
      found++;
      p++;
    }
    else if (isspace(c)){ }
    else
    {
      printf("invalid hex char '%c'\n", *s);
      exit(1);
    }
    s++;
  }

  //dump(ba);

  // if odd number of nibbles, then set p to ba.ptr, which will include the
  // padding nibble
  if (found & 0x1)
  {
    p = ba.ptr;
    ba.pad = ba.ptr;
    ba.len++;
    found++;
  }
  else
  {
    p = ba.ptr + 1;
  }

  // calc whole number of bytes, rounding if neccessay
  //int bytes = (found >> 1) + (found & 0x1);
  size_t bytes = found >> 1;

  byte_array ba2 = reserve(bytes);

  for (size_t i = 0; i < bytes; i++)
  {
    unsigned int v = (*(p+2*i) << 4) + *((p+2*i)+1);
    *(ba2.ptr + i) = v;
  }

  //dump(ba2);

  const int linewidth = 8; // bytes per line

  // TODO: would be nice to have the hexdump type display here
  // display the padding
  int blocks = bytes/linewidth + ( (bytes%linewidth)>0 );
  std::cout << "bytes:" << bytes << "\n";


  for (int j = 0; j < blocks; j++)
  {
    std::cout << j << ":"; // address

    int m = linewidth * j;
    std::cout << use_color( 1, THEME_HEX );
    for (size_t i = 0; i < (linewidth*2); ++i)
    {
      if (i%2 == 0) printf(" ");
      size_t hexindex = i+(m*2);
      if (hexindex < found)
      {
        if (ba.pad == (p+i+m*2)) std::cout << use_color( 1, THEME_PADOCTECT);
        std::cout << tohex( *(p+hexindex));
        if (ba.pad == (p+i+m*2))
        {
          std::cout << use_color( 1, THEME_RESET);
          std::cout << use_color( 1, THEME_HEX );
        }
      }
      else
      {
        printf(" ");
      }
    }
    printf(" |");
    for (size_t i = 0; i < linewidth; ++i)
    {
      if ((m+i) < bytes)
      {
        unsigned int b = *(ba2.ptr+m+i);
        printf(" %3u",b);
      }
    }

    std::cout << "\n";
  }


  // cast to int
  int t;
  memcpy(&t, ba2.ptr, sizeof(t));

  as_int( ba2 );

  return ba;
}

/*
            HI       LO
    65280 | 00 00 FF 00

    LO: this byte controls low range, eg +1
    HI: this byte conrols high reange, eg millions


    take int var, V:      [0][1][2][3]

    is four bytes.  Which of these has the LO, and which the HI?
 */

int main(int argc, char** argv)
{
  char * h = (char*)malloc(1);
  memset(h,0,1);

  for (int i = 1; i < argc; i++)
  {
    size_t needed = strlen(h) + strlen(argv[i]) + 1;
    h = (char*) realloc(h, needed);
    strcat(h, argv[i]);

  }

  hex_to_bytes( h );


  free(h);
  return 0;

}
