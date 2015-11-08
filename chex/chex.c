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

#include "color.h"

#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>  /* bool */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHEX_VERSION "1.0"

#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(!!(COND))*2-1]

// token pasting madness:
#define COMPILE_TIME_ASSERT3(X,L) STATIC_ASSERT(X,static_assertion_at_line_##L)
#define COMPILE_TIME_ASSERT2(X,L) COMPILE_TIME_ASSERT3(X,L)
#define COMPILE_TIME_ASSERT(X)    COMPILE_TIME_ASSERT2(X,__LINE__)


/*

TODO: improve the printable chars output

NEXT: allow user to hexdump the input bytes

NEXT: test options, and see if looks okay

NEXT: in the help, explain what LSB means

NEXT:  no diff in rsb lsb for : -t int AA BB CC DD

*/

/* Color and theme */
enum chex_theme
{
  THEME_RESET     = 0,
  THEME_HEX,
  THEME_PADOCTECT,
  THEME_PADBYTE,
  THEME_NONPRINT,
  THEME_HEADING
};

// map from theme enum to actual color to use
static char theme_colors[][COLOR_MAXLEN] = {
	GIT_COLOR_RESET,
	GIT_COLOR_NORMAL,	/* THEME_HEX */
	GIT_COLOR_RED,		/* THEME_PADOCTECT */
	GIT_COLOR_RED,		/* THEME_PADBYTE */
	GIT_COLOR_RED,		/* THEME_NONPRINT */
	GIT_COLOR_BOLD,		/* THEME_HEADING */
    "" // LAST
};

const char* use_color(bool wantcolor, int ix)
{
  if (wantcolor)
    return theme_colors[ix];
  else
    return GIT_COLOR_RESET;
}


/* Enum of supported types. This enum has same order and positions as the ctypes
 * array */
enum ectype
{
  CTYPE_CHAR = 0,
  CTYPE_SIGNED_CHAR,
  CTYPE_UNSIGNED_CHAR,
  CTYPE_SHORT,
  CTYPE_UNSIGNED_SHORT,
  CTYPE_INT,
  CTYPE_UNSIGNED_INT,
  CTYPE_LONG,
  CTYPE_UNSIGNED_LONG,
  CTYPE_LONG_LONG,
  CTYPE_UNSIGNED_LONG_LONG,
  CTYPE_FLOAT,
  CTYPE_DOUBLE,
  CTYPE_LONG_DOUBLE,

  CTYPE_MAX
};

struct ctype
{
  const char* name;
  const char* fmt;  // for printf
  int   use;
  int   is_char;
};

/* Supported types, and printf format string. This array has same order and
 * postion as the CTYPE_xxx enums. */
struct ctype ctypes[] =
{
  {"char",               "%hhi",    0, 1},  // assume signed, but is implementation defined
  {"signed char",        "%hhi",    0, 1},
  {"unsigned char",      "%hhu",    0, 1},
  {"short",              "%i",      0, 0},
  {"unsigned short",     "%hu",     0, 0},
  {"int",                "%i",      0, 0},
  {"unsigned int",       "%u",      0, 0},
  {"long",               "%li",     0, 0},
  {"unsigned long",      "%lu",     0, 0},
  {"long long",          "%lli",    0, 0},
  {"unsigned long long", "%llu",    0, 0},
  {"float",              "%.11g",   0, 0},
  {"double",             "%.16g",   0, 0},
  {"long double",        "%.19Lg",  0, 0},
  {0,0,0,0}  // final element must be empty
};

// static asserts
COMPILE_TIME_ASSERT( (CTYPE_MAX+1) == (sizeof(ctypes)/sizeof(struct ctype)));

struct program_options
{
  enum { PAD_LEFT=0, PAD_RIGHT} padside;
  bool showchars;
  bool alltypes;

  // Specifiies if byte is written as LSB/MSB convention (eg B1=28) . Default is
  // the MSB/LSB conventional (eg C1=193)
  bool byte_lsbmsb;

  // Reverse the final memcpy
  bool reverse;
};

static struct program_options popts;

//----------------------------------------------------------------------

struct byte_array
{
  // actual array
  void*  buf;
  size_t buflen;

  // region of user data
  unsigned char*  data;
  size_t datalen;
} bytes;


struct byte_array reserve(size_t len, size_t padlen)
{
  struct byte_array ba;
  memset(&ba, 0, sizeof(ba));

  ba.buflen = len + 2 * padlen;
  ba.buf    = malloc( ba.buflen );
  memset(ba.buf, 0, ba.buflen);

  ba.data = (unsigned char*) (ba.buf) + padlen;
  ba.datalen = len;

  return ba;
}

void dump(struct byte_array ba)
{
  for (size_t i = 0; i < ba.datalen; i++) printf("%u\n", *(ba.data+i));
  printf("\n");
}

bool ptr_in_user_region(const struct byte_array* ba, const unsigned char * p)
{
  return (p >= ba->data) && (p < ba->data + ba->datalen);
}

//----------------------------------------------------------------------

char tohex(unsigned int i)
{
  static const char * alphabet="0123456789ABCDEF";
  return alphabet[ i & 0xF];
}

//----------------------------------------------------------------------

bool is_little_endian()
{
   long int i = 1;
   const char *p = (const char *) &i;

   return (p[0] == 1);  // Lowest address contains the least significant byte
}

//----------------------------------------------------------------------

void endian_copy(void* dest,
                 unsigned char* src,
                 size_t n)
{
  int arch_little_endian = is_little_endian();
  bool user_litte_endian  = ( popts.padside==PAD_RIGHT);

  memcpy(dest,src,n);

  // if we need to, corrent for endian, or if user requested perform reverse;
  // but ignore need to correct for endian AND perform reverse
  if ( ((arch_little_endian != user_litte_endian)?1:0) ^ (popts.reverse?1:0) )
  {
    unsigned char* beg = dest;
    unsigned char* end = dest + n - 1;
    while(beg<end)
    {
      unsigned char temp = *beg;
      *beg = *end;
      *end = temp;
      beg++;
      end--;
    }
  }
}

//----------------------------------------------------------------------

/* Note the byte padding implemented in here.  If the hex string has
 * right-most-LSB, then padding will go to the left, to fill up higher
 * significant bytes; in practice that means moving our buffer start pointer to
 * a lower position that where the user data resides.
 */
#define as_TYPE2( T, CTYPE_ENUM )                                       \
  if (ctypes[CTYPE_ENUM].use || popts.alltypes)                         \
  {                                                                     \
    printf("%s%s (size %lu)%s\n", use_color(1, THEME_HEADING), #T, sizeof( T ), use_color(1, THEME_RESET)); \
    T value = 0;                                                        \
    const size_t width = sizeof( T );                                   \
    int pad = (width - ba.datalen % width) % width;                     \
    unsigned char* src = ba.data /* rightpad */;                        \
    if (popts.padside==PAD_LEFT)                                        \
    {                                                                   \
      src = ba.data - pad;                                              \
    }                                                                   \
    int values = (ba.datalen + pad) / width;                            \
    for (int j = 0; j< values; ++j)                                     \
    {                                                                   \
      /*memcpy(&value, src, width);*/                                   \
      endian_copy(&value, src, width);                                 \
      /* display hex */                                                 \
      for (size_t i = 0; i < width; ++i)                                \
      {                                                                 \
        bool b = !ptr_in_user_region(&ba,src+i);                        \
        printf(" %s%02X%s", use_color(1 && b, THEME_PADBYTE), *(src+i), use_color(1 && b, THEME_RESET)) ; \
      }                                                                 \
      printf(" | ");                                                    \
      printf(ctypes[CTYPE_ENUM].fmt , value);                           \
      if (ctypes[CTYPE_ENUM].is_char  && isprint( value )) printf(" '%c'", (char)value); \
      printf("\n");                                                     \
      src += width;                                                     \
    }                                                                   \
  }

//----------------------------------------------------------------------

void convert_to_types(struct byte_array ba)
{
  as_TYPE2(char,   CTYPE_CHAR);
  as_TYPE2(signed char,   CTYPE_SIGNED_CHAR);
  as_TYPE2(unsigned char,   CTYPE_UNSIGNED_CHAR);

  as_TYPE2(short,  CTYPE_SHORT);
  as_TYPE2(unsigned short,  CTYPE_UNSIGNED_SHORT);

  as_TYPE2(int, CTYPE_INT);
  as_TYPE2(unsigned int, CTYPE_UNSIGNED_INT);

  as_TYPE2(long, CTYPE_LONG);
  as_TYPE2(unsigned long, CTYPE_UNSIGNED_LONG);

  as_TYPE2(long long, CTYPE_LONG_LONG);
  as_TYPE2(unsigned long long, CTYPE_UNSIGNED_LONG_LONG);

  as_TYPE2(float, CTYPE_FLOAT);
  as_TYPE2(double, CTYPE_DOUBLE);
  as_TYPE2(long double, CTYPE_LONG_DOUBLE);


  if (popts.showchars)
  {
    printf("%sprintable chars%s\n", use_color(1, THEME_HEADING), use_color(1, THEME_RESET));
    const char* col_nonprint=use_color(1, THEME_NONPRINT);
    const char* col_reset=use_color(1, THEME_RESET);

    unsigned char* src = ba.data;
    for (size_t i = 0; i < ba.datalen; ++i, ++src)
    {
      if (isprint( *src ))
        printf("%c", *src);
      else if (col_nonprint != col_reset)
        printf("%s.%s", col_nonprint,col_reset);
    }
    printf("\n");
  }
}

//----------------------------------------------------------------------

/* convert the user hex-string into an array of bytes */
void process_user_hex_string(const char* s)
{
  const int slen = strlen(s);
  if (slen ==0 ) return;

  // Allocate a buffer to hold the integer values of each nibble.  Length is +1,
  // because we will need to pad the user supplied hex sequence if an odd number
  // of hex characters were provided.
  size_t const nibblebuflen = slen+1;
  unsigned char * nibblesbuf = (unsigned char*) malloc( nibblebuflen );
  unsigned char * nibblesptr = nibblesbuf;

  for (int i = 0; i < slen;i++)
  {
    char c = tolower(*(s+i));

    if (isdigit(c))
    {
      *nibblesptr = c - '0';
      nibblesptr++;
    }
    else if (c>='a' && c <= 'f')
    {
      *nibblesptr = 10+(c-'a');
      nibblesptr++;
    }
    else if (isspace(c)){ }
    else
    {
      printf("invalid hex char '%c'\n", *(s+i));
      exit(1);
    }
  }
  int nibbles = nibblesptr-nibblesbuf;

  // If user supplied odd number of nibbles, then apply a padding if needed and
  // if we are to pad on the left hand side (ie requires a memory move).
  if (nibbles & 0x01)
  {
    if (popts.padside==PAD_LEFT)
    {
      memmove(nibblesbuf+1, nibblesbuf, nibbles);
      *nibblesbuf = 0;  // padding on left
    }
    else
      nibblesbuf[ nibbles ] = 0; // padding on right
  }

  const int bytes = (nibbles+1) >> 1;

  /* convert nibble pairs into bytes */
  struct byte_array ba = reserve( bytes, 32 ); // constant is pad bytes
  for (int i = 0; i < bytes; i++)
  {
    unsigned char n1 = nibblesbuf[i*2];
    unsigned char n2 = nibblesbuf[i*2+1];
    *(ba.data+i) = popts.byte_lsbmsb? (n2 << 4) + n1 : (n1 << 4) + n2;
  }
  //dump(ba);

  /* display the raw bytes */

  const int linewidth = 8; // bytes per line
  const int blocks = bytes/linewidth + ( (bytes%linewidth)>0 );
  printf("nibbles %u, bytes %u\n", nibbles, bytes);
  for (int j = 0; j < blocks; j++)
  {
    printf("%d:", j);
    int offset = linewidth * j;

    // loop over each byte, convert to hex, print
    for (size_t i = 0; i < linewidth ; ++i)
    {
      if ((offset+i) < bytes)
      {
        unsigned int b = *(ba.data+offset+i);

        // Note: there are easier ways to get hex digits, but, this is okay for
        // now, becuase I might display the hex in different colours.
        char msb = tohex((b & 0xF0)>>4);
        char lsb = tohex((b & 0x0F));
        printf(" %c%c",msb,lsb);
      }
      else printf("   ");
    }
    printf(" |");

    for (size_t i = 0; i < linewidth && ((offset+i) < bytes); ++i)
    {
      unsigned int b = *(ba.data+offset+i);
      printf(" %3u",b);
    }
    printf("\n"); // newline after one block
  }

  free(nibblesbuf);

  convert_to_types( ba ) ;
}

//----------------------------------------------------------------------
void usage()
{
  printf("chex - convert hex strings into values of C/C++ primitive types\n");
  printf("\nUsage: chex OPTIONS HEX [HEX ...]\n\n");
  printf("  -l, --leftpad  \tadd padding to left side of input bytes, implies LSB on right (default)\n");
  printf("  -r, --rightpad \tadd padding to right side of input bytes, implies LSB on left\n");
  printf("  -t, --types    \tcomma separated list of C types (see below), or 'all'\n");
  printf("  --reverse      \treverse final memcpy from byte array to primitive type\n");
  printf("  --lsbmsb       \tinterpret hex chars as LSB-MSB pairs, instead of MSB-LSB convention\n");
  printf("  -v             \tshow version\n");
  printf("\nTypes supported:\n\n");
  struct ctype * s = ctypes;
  while (s->name)
  {
    printf("  %s\n", s->name);
    s++;
  }

  const char* longhelp =
    "Converting the user hex string to values of C/C++ types (such as int, double\n"
    "etc) is a three step process.\n"
    "\n"
    "1. Hex pairs to bytes\n"
    "\n"
    "The hex string is first converted into a sequence of octects (bytes). This\n"
    "conversion, which operates on pairs of hex digits, depends on which digit\n"
    "contains the most significant bit (MSB).  For example the hex number A2 can be\n"
    "interpreted in two ways, depending on the order MSB-LSB or LSB-MSB :\n"
    "\n"
    "    MSB-LSB order :  10*16 + 2     =  162\n"
    "    LSB-MSB order :  10    + 2*16  =  42\n"
    "\n"
    "chex uses MSB-LSB order by default; the program option --lsbmsb allows for\n"
    "reverse ordering.\n"
    "\n"
    "2. Padding\n"
    "\n"
    "Padding is the introduction of zero bytes in higher-significant positions to\n"
    "fill in gaps when not enough input hex digits have been provided to convert into\n"
    "a C primitive type.\n"
    "\n"
    "For example, converting the hex string '1A2' into a 4 byte unsigned int can be\n"
    "performed in two different ways, depending on whether padding is placed to the\n"
    "left or to the right of the full hex string.  In both cases, the padding is\n"
    "always assumed to be the higher order bytes.\n"
    "\n"
    "* using left padding (-l option, the default)\n"
    "\n"
    "     user string :   1A2\n"
    "\n"
    "     padded      :   000001A2\n"
    "\n"
    "     hex to bytes:   00  00  01  A2\n"
    "                      0   0   1 162\n"
    "                    MSB\n"
    "\n"
    "     uint value  :   418\n"
    "\n"
    "* using right padding (-r option)\n"
    "\n"
    "     user string :   1A2\n"
    "\n"
    "     padded      :   1A200000\n"
    "\n"
    "     hex to bytes:   1A  20  00  00\n"
    "                     26  32   0   0\n"
    "                                 MSB\n"
    "\n"
    "     uint value  :   32*256 + 26  = 8218\n"
    "\n"
    "3. Endian\n"
    "\n"
    "The final step is the copy of bytes into a C primitive type.  The left or right\n"
    "padding setting determines which user byte is the LSB and which is the MSB; this\n"
    "information allows the bytes to be copied in an order which reflects the\n"
    "endianess of the host system. I.e., the LSB user byte will occupy the LSB\n"
    "location in the C/C++ type.\n" ;

  printf("\nNotes:\n\n");
  printf(longhelp);

  exit(0);
}


//----------------------------------------------------------------------
void version()
{
  printf("chex %s\n", CHEX_VERSION);
  exit(0);
}

//----------------------------------------------------------------------
/* Option processing */

bool isopt(const char* opt1, const char* opt2, const char* user)
{
  return (opt1 && (strcmp(opt1,user))==0) || (opt2 && (strcmp(opt2,user)==0));
}

int isoptarg(const char* opt1, const char* opt2, int argc, char** argv, int *i)
{
  if ((strcmp(opt1,argv[*i])==0) || (strcmp(opt2,argv[*i])==0))
  {
    if ((*i+1) < argc)
    {
      *i = *i + 1;
      return 1;
    }
    else
    {
      printf("missing argument for: %s\n", argv[*i]);
      exit(1);
    }
  }
  return 0;
}

int opt_unknown(int argc, char** argv, int *i)
{
  if (*i < argc && argv[*i][0] == '-')
  {
    printf("unknown option '%s'\n", argv[*i]);
    exit(1);
  }
  return 0;
}

//----------------------------------------------------------------------

int main(int argc, char** argv)
{
  char * usertypes = 0;

  popts.showchars = true;  // TODO: take from user

  // parse options
  int i;
  for (i = 1; i < argc; i++)
  {
    if      ( isopt("-h", "--help",     argv[i]) ) usage();
    else if ( isopt("-v", "--version",  argv[i]) ) version();
    else if ( isopt("-l", "--leftpad",  argv[i]) ) popts.padside     = PAD_LEFT;
    else if ( isopt("-r", "--rightpad", argv[i]) ) popts.padside     = PAD_RIGHT;
    else if ( isopt(NULL, "--lsbmsb",   argv[i]) ) popts.byte_lsbmsb = true;
    else if ( isopt(NULL, "--reverse",  argv[i]) ) popts.reverse     = true;
    else if ( isoptarg("-t", "--types", argc, argv, &i))
    {
      /* strdup is not alway available, so copy here*/
      free(usertypes);  // just in case of multiple -t options
      size_t __len = strlen(argv[i])+1;
      usertypes = malloc(__len);
      memcpy(usertypes, argv[i], __len);
    }
    else if ( opt_unknown(argc, argv, &i) ) {}
    else break;
  }

  /* process the types argument */
  char * fd = strtok(usertypes, ",");
  while(fd)
  {
    int found = 0;

    if (strcmp(fd,"all")==0)
    {
      popts.alltypes=1;
      found = 1;
    }
    else for (int j = 0; ctypes[j].name && !found; j++)
    {
      if ((strcmp(fd,ctypes[j].name) == 0))
      {
        ctypes[j].use = 1;
        found = 1;
      }
    }

    if (!found)
    {
      printf("'%s' is not a supported type\n", fd);
      exit(1);
    }
    fd = strtok(NULL, ",");
  } // while


  // concatenate the remaining command lines args which represent the hex digits
  // provided by the user
  char * h = (char*)malloc(1);
  *h='\0';

  for (; i < argc; i++)
  {
    size_t needed = strlen(h) + strlen(argv[i]) + 1;
    h = (char*) realloc(h, needed);
    strcat(h, argv[i]);
  }

  process_user_hex_string( h );

  free(h);
  free(usertypes);
  return 0;
}
