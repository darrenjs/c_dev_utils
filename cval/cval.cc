
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <list>
#include <bitset>
#include <limits>
#include <cstring>
#include <stdexcept>

#include <stdlib.h>


// helper macros to support string concatenation
#define CVAL_CONCAT2( a, b ) a##b
#define CVAL_CONCAT( a, b ) CVAL_CONCAT2( a, b )

/* Template function for getting the name of a type. This is specialised later
 * to provide a set of functions that converts a type to its string, for
 * example: int --> "int"
 */
template <typename T> const char* name_of_type();

/* Get hex representation of a value */
template<typename T>
void to_hex(T _v , std::ostream& s)
{
  union
  {
      T value;
      unsigned char bytes[sizeof(T)];
  };
  memset(&bytes, 0, sizeof(T));
  value = _v;

  s << std::setfill('0');
  s << std::hex;
  s << std::uppercase;
  s << std::right;

#if BYTE_ORDER == BIG_ENDIAN
  for (int i = 0; i < sizeof(T); i++) {
#else
  for (int i = sizeof(T)-1; i >= 0; i--) {
#endif
    s << " " << std::setw(2) << (unsigned int) bytes[i];
  }
}

/*
 * Get binary representation of a value.  We use std::bitset to generate
 * the actual binary for each byte.
 */
template <typename T>
void to_bin(T _v, std::ostream& os)
{
  std::bitset< std::numeric_limits<unsigned char>::digits > bs;
  union
  {
      T value;
      unsigned char bytes[sizeof(T)];
  };
  memset(&bytes, 0, sizeof(T));
  value = _v;

#if BYTE_ORDER == BIG_ENDIAN
  for (int i = 0; i < sizeof(T); i++)
#else
  for (int i = sizeof(T)-1; i >= 0; i--)
#endif
  {
    bs = bytes[i];
    os << " " << bs;
  }

}

/*
 * Utility methods for a primitive type.
 */
template <typename T>
struct typeutil
{
    typedef T io_type;

    static T from_string(const char* s,
                         std::ios_base& (*base)(std::ios_base&))
    {
      T retval = T();

      std::istringstream iss(s);

      if ((iss >> base >> retval).fail())
      {
        std::ostringstream oss;
        oss << "failed to convert " << s << " to "
            << ::name_of_type<T>();
        throw std::ios_base::failure(oss.str());
      }

      return retval;
    }
};

/* ===== Specialisation for char types ===== */

/*
 * Generic method to use when the conversion from a string to a type
 * needs to go via an intermediate ordinal type.
 */
template <typename S, typename I>
S convert_via_int(const char* s,
                  std::ios_base& (*base)(std::ios_base&))
{
  std::istringstream iss(s);

  I intermediate;

  if ((iss >> base >> intermediate).fail())
  {
    std::ostringstream oss;
    oss << "failed to convert \"" << s << "\" to "
        << ::name_of_type<S>();
    throw std::ios_base::failure(oss.str());
  }

  return intermediate;
}

template <>
struct typeutil<char>
{
  typedef long io_type;
  static char from_string(const char* s,
                          std::ios_base& (*base)(std::ios_base&));
};

template <>
struct typeutil<unsigned char>
{
  typedef unsigned long io_type;
    static unsigned char from_string(const char* s,
                                     std::ios_base& (*base)(std::ios_base&));
};

template <>
struct typeutil<signed char>
{
  typedef signed long io_type;
  static signed char from_string(const char* s,
                                 std::ios_base& (*base)(std::ios_base&));
};

/* Formatted output of a floating point type value */
template <typename T>
void pr_float(T v, std::ostream& os, int p, int e, bool pad)
{
  os << std::dec;
  if (pad) os << std::setw(2+p+2+e);
  os << std::right
     << std::setfill(' ')
     << std::setprecision(p)
     << v;
}

/* Formatted output of a value.  This function is specialised (below)
 * for floating point types. */
template <typename T>
void pr_raw(T v, std::ostream& os, bool pad)
{
  typename typeutil<T>::io_type o = v;

  if (pad)
  {
    os << std::setw( 1+std::numeric_limits<T>::digits10 + // add 1 for most-sig. position
                     std::numeric_limits<T>::is_signed);  // add 1 for minus sign
  }
  os << std::dec << std::right << std::setfill(' ') << o;
}
template<> void pr_raw(float v, std::ostream& os, bool pad) { pr_float(v, os, 20, 5, pad); }
template<> void pr_raw(double v, std::ostream& os, bool pad){ pr_float(v, os, 20, 5, pad); }
template<> void pr_raw(long double v, std::ostream& os, bool pad){ pr_float(v, os, 25, 6, pad); }


/* For a type T, and a set of values starting at argv[argoffset],
 * display a summary of the type and the hex and binary
 * representations of the set of values. */
template <typename T>
void dump(int argc, const char** argv, int argoffset, std::ostream& os)
{
  // summary line
  os << name_of_type<T>()
     << ", sizeof " << sizeof(T)
     << ", non-sign-bits " << std::numeric_limits<T>::digits
     << ", digits10 " << std::numeric_limits<T>::digits10
     << ", min ";
  pr_raw<T>(std::numeric_limits<T>::min(), os, false);
  os << ", max ";
  pr_raw<T>(std::numeric_limits<T>::max(), os, false);
  os << std::endl;

  // display the bin & hex representations of the values
  for (int i = argoffset; i < argc; ++i)
  {
    T f = typeutil<T>::from_string(argv[i], std::dec);
    pr_raw<T>(f, os, true);
    os << " |";
    ::to_hex<T>(f, os);
    os << " |";
    to_bin<T>(f, os);
    os << std::endl;
  }
}


typedef void (*dump_funptr)(int, const char**, int, std::ostream&);
std::map<std::string, dump_funptr> dump_funs;
std::list<std::string> supported_types;

template< typename T >
struct TypeDecoder
{

  TypeDecoder()
  {
    // register
    dump_funs[ ::name_of_type< T >() ] = TypeDecoder<T>::dump;
    supported_types.push_back( ::name_of_type< T >() ) ;
  }

  static void dump(int argc, const char** argv, int argoffset, std::ostream& os)
  {
    ::dump<T>(argc, argv, argoffset, os);
  }
};


/* Note a little trick here.
 */
#define GENERATE_CONVERTER( T )                                 \
  template <> const char* name_of_type< T >() { return #T; }    \
  TypeDecoder< T > CVAL_CONCAT( __decoder_ , __LINE__ );

// Generate decoders for the usual family of C/C++ types.  Not that types like
// "long int" are not here; they are represented as just "long".
GENERATE_CONVERTER( bool );
GENERATE_CONVERTER( double );
GENERATE_CONVERTER( long double );
GENERATE_CONVERTER( float );
GENERATE_CONVERTER( char );
GENERATE_CONVERTER( signed char );
GENERATE_CONVERTER( unsigned char );
GENERATE_CONVERTER( int );
GENERATE_CONVERTER( unsigned int );
GENERATE_CONVERTER( long );
GENERATE_CONVERTER( unsigned long );
GENERATE_CONVERTER( long long );
GENERATE_CONVERTER( unsigned long long );
GENERATE_CONVERTER( short );
GENERATE_CONVERTER( unsigned short );

/* Next follows the definition of the from_string methods for the char variants
 * of the typeutil class. These need to come after the generation of convertors,
 * so that older version of GCC (eg 3.5) don't complain about the template
 * already being initialised.
 */
char typeutil<char>::from_string(const char* s,
                                 std::ios_base& (*base)(std::ios_base&))
{
  return ::convert_via_int<char, io_type>(s, base);
}
unsigned char typeutil<unsigned char>::from_string(const char* s,
                                                   std::ios_base& (*base)(std::ios_base&))
{
  return ::convert_via_int<unsigned char, io_type>(s, base);
}
signed char typeutil<signed char>::from_string(const char* s,
                                               std::ios_base& (*base)(std::ios_base&))
{
  return ::convert_via_int<signed char, io_type>(s, base);
}

void usage()
{
  std::cout << "cval\n";

  std::cout << "Supported types:" << std::endl;
  for (std::list<std::string>::iterator i=supported_types.begin();
       i != supported_types.end(); ++i)
    std::cout << "\t" << *i << "\n";

  std::cout << std::endl << "Compiled on ILP:" << std::endl
            << "\tint     " << sizeof(int)   << std::endl
            << "\tlong    " << sizeof(long)  << std::endl
            << "\tpointer " << sizeof(void*) << std::endl;
  ::exit(0);
}


int __main(int argc, const char** argv)
{
  std::string usertype;
  int i=1;
  for (; i < argc; i++)
  {
    if ( (strcmp(argv[i],"-h")==0) || (strcmp(argv[i],"--help")==0) ) usage();

    if (argv[i][0] >= 'a' && argv[i][0] <= 'z')
    {
      usertype += argv[i];
      usertype.resize(usertype.length()+1, ' ');
    }
    else
      break;
  }

  if (!usertype.empty())
    usertype.resize(usertype.length()-1);
  else
    usage();

  dump_funptr fptr = dump_funs[ usertype ];
  if (!fptr) throw std::runtime_error("type not supported");

  fptr(argc, argv, i, std::cout);

  return 0;
}


int main(int argc, const char** argv)
{
  try
  {
    return __main(argc,argv);
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "unknown error" << std::endl;
  }
  return 2;
}
