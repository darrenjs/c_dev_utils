
#include "color.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>  /* bool */
#include <getopt.h>
#include <assert.h>

#define CHEX_VERSION "1.0"

#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(!!(COND))*2-1]
// token pasting madness:
#define COMPILE_TIME_ASSERT3(X,L) STATIC_ASSERT(X,static_assertion_at_line_##L)
#define COMPILE_TIME_ASSERT2(X,L) COMPILE_TIME_ASSERT3(X,L)
#define COMPILE_TIME_ASSERT(X)    COMPILE_TIME_ASSERT2(X,__LINE__)


/*

TODO: decide on endian:


so, take principle that padding drives endian selection; i.e., the padding bytes
are the MSB and non padding are LSB.

so we now have bytes, and side which is little endian


--




next dilemma, think about th eendian

1/ uiser input:   C1 00

2/  user says "pad left"  ==>  00 00 C1 00

3/ or, pad right @   C1 00 00 00

4/ so, for rightpad, and leftpad, how do we do the copy to the actual int?

5/ One option, is to take the left/right-ness, as a guide to which LSB, and if
we know the platform is little endian, then we can do the match

3/ now program does the split, starting wiht the non-pad end:  | 00

TODO: add option to provide endian to program, ie, assume-little, assume-big

TODO: improve the printable chars output

TODO: support computer endian, but where? I think I still need to put it in the  to byte conversion.

NEXT: allow user to hexdump the input bytes

NEXT: test options, and see if looks okay


NEXT: in the help, explain what LSB means

NEXT:  no diff in rsb lsb for : -t int AA BB CC DD

TODO: improve the help output



* need to rememebr that we are taking long bytes sequences from the user

     02 03 04 05 | 06 07 08 09 | AA BB CC DD | EE FF

    02 03 04 05 | 06 07 08 09 | AA BB CC DD | EE FF  00 00

or

     00 00 02 03 | 04 05 06 07 | 08 09 AA BB | CC DD EE FF

NOTE: padding is not needed if we have correct number of multiple bytes


Here is the problem.  Take this string:

       0F 0A 01

... now what is this equivalent to?

      3     2     1   0
     00    0F    0A   01
     00    01    0A   0F
     00    10    A0   F0

so, as well as determine which side of the string contains the LSB, it also
controls which side of the string should be padded (the side with the MSB).
Ie., padding is required here because we are converting to int size 4, but nly
have 3 bytes.

So, we need to ask the user: is string rightside-lsb (default), or, leftside-lsb?


the new pad algo:


   ba:   00 00 00 | 0F 0A 01 | 00 00 00
         ^^^^^^^^
         margin

so, easiest is to adaopt a little endian layout in memory.




NEXT: improve

the LSB and RSB options specify how to interpret the user hex string by
identifying which end of the string represents the least significant byte.  For
example, fiven the user string "FF AA", this is would be intpreted as follows:

    chex            "FF AA"

    right lsb        00 00 FF AA  =  65450
    left lsb         FF AA 00 00  = 4289331200   <-- wrong, should be    00 00 AA FF




    --rightpad      255  1    = 65450
    --leftpad       255  1    = 65450

                1       256    65536     --left

  in dislpay, we use positional format of putting the least sig bits on the right, and most sig bits on the left.

  - user says if input of big endian, or little endian, and that has definite
    meaning ... implies location of msb and lsb

  - put this in the help

  - padding is always on the msb side



NEXT: remove references to git

NEXT: add option to disable colour

NEXT: try to clean up the code




* need to think about the best way to present the padding and endian-ness


 * understand the endian issue

* support strings

* extract a string library?


* try to export the color lib

*/



/* Color and theme */
enum chex_theme
{
  THEME_RESET     = 0,
  THEME_HEX,
  THEME_PADOCTECT,
  THEME_PADBYTE,
  THEME_NONPRINT
};

// map from theme enum to actual color to use
static char theme_colors[][COLOR_MAXLEN] = {
	GIT_COLOR_RESET,
	GIT_COLOR_NORMAL,	/* THEME_HEX */
	GIT_COLOR_RED,		/* THEME_PADOCTECT */
	GIT_COLOR_RED,		/* THEME_PADBYTE */
	GIT_COLOR_RED,		/* THEME_NONPRINT */
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
  enum { PAD_RIGHT=0, PAD_LEFT} padside;
  bool showchars;
  bool alltypes;

  // Specifiies if byte is written as LSB/MSB convention (eg B1=28) . Default is
  // the MSB/LSB conventional (so C1=193)
  bool byte_lsbmsb;
} popts;



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


char tohex(unsigned int i)
{
  static const char * alphabet="0123456789ABCDEF";
  return alphabet[ i & 0xF];
}




/* endian_copy2 */

/* - src */
/* - bool archIsLittle */
/* - if leftpad, userbyteIsLittle=false */
/* - if rightpad,  userbyteIsLitter=true */
/* - mempy */
/* - if endian differnet, reverse */

bool is_little_endian()
{
   long int i = 1;
   const char *p = (const char *) &i;

   return (p[0] == 1)  // Lowest address contains the least significant byte
}

void endian_copy2(void* dest,
                 unsigned char* src,
                 size_t n)
{
  bool arch_little_endian = is_little_endian();
  bool user_litte_endian = ( pots.padside==PAD_RIGHT );

  memcpy(dest,src,n);

  if (arch_little_endian != user_litte_endian)
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

/* Copy bytes from src into dest.  Number of bytes to copy is passed in
 * 'width'.
 */
void endian_copy(void* dest,
                 unsigned char* src,
                 size_t n)
{
  unsigned char* tmpbuf = (unsigned char*) malloc(n);
  memcpy(tmpbuf, src, n);

  unsigned char* beg = tmpbuf;
  unsigned char* end = tmpbuf + n - 1;
  while(beg<end)
  {
    unsigned char temp = *beg;
    *beg = *end;
    *end = temp;
    beg++;
    end--;
  }

  // TODO: if endian match
  memcpy(dest, tmpbuf, n);

  free(tmpbuf);
}


/* Note the byte padding implemented in here.  If the hex string has
 * right-most-LSB, then padding will go to the left, to fill up higher
 * significant bytes; in practice that means moving our buffer start pointer to
 * a lower position that where the user data resides.
 */
#define as_TYPE2( T, CTYPE_ENUM )                                       \
  if (ctypes[CTYPE_ENUM].use || popts.alltypes)                         \
  {                                                                     \
    printf("%s (%lu bytes)\n", #T, sizeof( T ));                        \
    T value = 0;                                                        \
    const size_t width = sizeof( T );                                   \
    int pad = (width - ba.datalen % width) % width;                     \
    unsigned char* src = ba.data /* rightpad */;                        \
    if (popts.padside==PAD_RIGHT)                                       \
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
    printf("printable chars\n");
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
    if (popts.padside==PAD_RIGHT)
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


void usage()
{
  printf("chex - convert hex strings into values of C/C++ primitive types\n");
  printf("\nUsage: chex OPTIONS HEX [HEX ...]\n\n");
  printf("  -l, --leftpad  \tassume input has LSB on left side\n");
  printf("  -r, --rightpad \tassume input has LSB on right side (default)\n");
  printf("  -t, --types    \tcomma separated list of types, or 'all'\n");
  printf("  --lsbmsb       \tread hex chars as LSB-MSB pairs, instead of MSB-LSB convention\n");
  printf("  -v             \tshow version\n");
  printf("\nValues supported for the types option:\n");
  struct ctype * s = ctypes;
  while (s->name)
  {
    printf("  %s\n", s->name);
    s++;
  }
  printf("NOTES\n");
  printf("NOTES\n");

  /*
    TODO: here explain about the MSB/LSB dilemma

ie,   [LSB][MSB]  or [MSB][LSB]

    TODO: also explain about the padding

    ie, padding etc.   [bbbb bbbb b000 0000000] or [0000000 000bb bbbb bbbb]

    ie, next we need to divide into 4's (int).

                M  L M  L M  L
       [0000000 00bb bbbb bbbb] --> 'bbbb' to int(iiii)

   ok, should have enough info at this point. And internally, this large array
   will be stored in user side at lower end of range, and padding bytes at upper end of range.

 */

  exit(0);
}


void version()
{
  printf("chex %s\n", CHEX_VERSION);
  exit(0);
}

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
  char * usertypes = 0;

  memset(&popts,0,sizeof(struct program_options));

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
