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

#include <bitset>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <sstream>
#include <stdexcept>

#include <stdlib.h>

#define CVAL_VERSION "1.0"

// helper macros to support string concatenation
#define CVAL_CONCAT2( a, b ) a##b
#define CVAL_CONCAT( a, b )  CVAL_CONCAT2( a, b )

/* Template function for converting a type to its name. This is specialised
 * later to provide a set of functions that converts a type to its string, for
 * example: int --> "int"
 */
template <typename T> const char* name_of_type();

/* Genereate hex representation of a value */
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

  s << std::setfill('0') << std::hex
    << std::uppercase    << std::right;

#if BYTE_ORDER == BIG_ENDIAN
  for (int i = 0; i < sizeof(T); i++) {
#else
  for (int i = sizeof(T)-1; i >= 0; i--) {
#endif
    s << " " << std::setw(2) << (unsigned int) bytes[i];
  }
}

/*
 * Generate binary representation of a value.  Uses std::bitset to generate the
 * actual binary for each byte.
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
 * Generic from string conversion method, for converting from a string to a
 * value of type T, using C++ stream operations.  Supports an intermediate
 * convertion type for 'char' types.
 */
template <typename T, typename I>
static T convert_via(const char* s,
                       std::ios_base& (*base)(std::ios_base&))
{
  std::istringstream iss(s);

  // the intiial string conversion, using streams, goes via thhis temp
  // varialbe.
  I temp = I();

  if ((iss >> base >> temp).fail())
  {
    std::ostringstream oss;
    oss << "failed to convert \"" << s << "\" to "
        << ::name_of_type<T>();
      throw std::ios_base::failure(oss.str());
  }

  // final part of conversion
  return (T) temp;
}

/*
 * Utility method to convert from a string to a value of type T
 */
template <typename T>
struct FromString
{
  typedef T dest_type;  // destination type

  typedef T io_type;    // intermediate type

  // default conversion operation - this is specialised for particular types
  static T convert(const char* s,
                   std::ios_base& (*base)(std::ios_base&))
  {
    return ::convert_via<T,T>(s, base);
  }

};

/* Specialisation for char types  */

template <>
struct FromString<char>
{
  typedef char dest_type;
  typedef long io_type;

  static dest_type convert(const char* s,
                           std::ios_base& (*base)(std::ios_base&));
};

template <>
struct FromString<unsigned char>
{
  typedef unsigned char dest_type;
  typedef unsigned long io_type;

  static dest_type convert(const char* s,
                           std::ios_base& (*base)(std::ios_base&));
};

template <>
struct FromString<signed char>
{
  typedef signed char dest_type;
  typedef signed long io_type;

  static dest_type convert(const char* s,
                           std::ios_base& (*base)(std::ios_base&));
};

/* Formatted output of a floating point type value */
template <typename T>
void pr_float(T v, std::ostream& os, int prec, int e, bool pad)
{
  os << std::dec;
  if (pad) os << std::setw(2+prec+2+e);
  os << std::right
     << std::setfill(' ')
     << std::setprecision(prec)
     << v;
}

/* Formatted output of a value.  This function is specialised (below)
 * for floating point types. */
template <typename T>
void pr_raw(T v, std::ostream& os, bool pad)
{
  typename FromString<T>::io_type o = v;

  if (pad)
  {
    os << std::setw( 1+std::numeric_limits<T>::digits10 + // add 1 for most-sig. position
                     std::numeric_limits<T>::is_signed);  // add 1 for minus sign
  }
  os << std::dec << std::right << std::setfill(' ') << o;
}

// template specialisations for real types
template<> void pr_raw(float v, std::ostream& os, bool pad)
{
  pr_float(v, os, 20, 5, pad);
}
template<> void pr_raw(double v, std::ostream& os, bool pad)
{
  pr_float(v, os, 20, 5, pad);
}
template<> void pr_raw(long double v, std::ostream& os, bool pad)
{
  pr_float(v, os, 25, 6, pad);
}


/* For a type T, and a set of values starting at argv[argoffset],
 * display a summary of the type and the hex and binary
 * representations of the set of values. */
template <typename T>
void dump_impl(int argc, const char** argv, int argoffset, std::ostream& os)
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
    T f = FromString<T>::convert(argv[i], std::dec);
    pr_raw<T>(f, os, true);
    os << " |";
    ::to_hex<T>(f, os);
    os << " |";
    to_bin<T>(f, os);
    os << std::endl;
  }
}


typedef void (*dump_funptr)(int, const char**, int, std::ostream&);

// global functions that list the supported types and their hanlder functions.
std::map<std::string, dump_funptr> dump_funcs;
std::list<std::string> supported_types;


/*
 * TypeHandler class provides a mechanism for wrapping the dump_impl method, and
 * for registering the method when an class instance is created.
 */
template< typename T >
struct TypeHandler
{
  TypeHandler()
  {
    // register with global containers
    dump_funcs[ ::name_of_type< T >() ] = TypeHandler<T>::dump;
    supported_types.push_back( ::name_of_type< T >() ) ;
  }

  static void dump(int argc, const char** argv,
                   int argoffset, std::ostream& os)
  {
    ::dump_impl<T>(argc, argv, argoffset, os);
  }
};


/* Macro to generate a type handler. There is a little trick in here. A type
 * handler is created by defining an instance of TypeHandler<T>. Each such
 * instance needs a unique variable name, and these names are generated by
 * incorparating the line number, from where the macro is invoked, into the
 * concantenated variable name.
 */
#define GENERATE_CONVERTER( T )                                     \
  template <> const char* name_of_type< T >() { return #T; }        \
  TypeHandler< T > CVAL_CONCAT( __decode_instance_ , __LINE__ );

// Invoke generator the usual family of C/C++ types.  Not that types like "long
// int" are not here; they are represented as just "long". This is the only
// place in the program where the types are listed.
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


/* The definition of the convert methods for the char variants of the
 * FromString class. These need to come after the generation of convertors, so
 * that older version of GCC (eg 3.5) don't complain about the template already
 * being initialised.
 */
char FromString<char>::convert(const char* s,
                               std::ios_base& (*base)(std::ios_base&))
{
  return ::convert_via<dest_type,io_type>(s, base);
}
unsigned char FromString<unsigned char>::convert(const char* s,
                                                 std::ios_base& (*base)(std::ios_base&))
{
  return ::convert_via<dest_type, io_type>(s, base);
}
signed char FromString<signed char>::convert(const char* s,
                                             std::ios_base& (*base)(std::ios_base&))
{
  return ::convert_via<dest_type, io_type>(s, base);
}


void version()
{
  std::cout << "cval " CVAL_VERSION " (";
#if defined(__LP64__) || defined(_LP64)
  std::cout << "64";
#else
  std::cout << "32";
#endif
  std::cout << " bit mode)\n";
  ::exit(0);
}

void usage()
{
  std::cout << "Usage: cval [-hv] TYPE VALUE...\n\n";

  std::cout << "Supported types:" << std::endl;
  for (std::list<std::string>::iterator i=supported_types.begin();
       i != supported_types.end(); ++i)
    std::cout << "\t" << *i << "\n";

  std::cout << std::endl << "Compiled on ILP:" << std::endl
            << "\tint     " << sizeof(int)     << std::endl
            << "\tlong    " << sizeof(long)    << std::endl
            << "\tpointer " << sizeof(void*)   << std::endl;
  ::exit(0);
}


int __main(int argc, const char** argv)
{
  std::string usertype;
  int i=1;
  for (; i < argc; i++)
  {
    if ( (strcmp(argv[i],"-h")==0) || (strcmp(argv[i],"--help")==0) ) usage();
    if ( (strcmp(argv[i],"-v")==0) || (strcmp(argv[i],"--version")==0) ) version();

    if (argv[i][0] >= 'a' && argv[i][0] <= 'z')
    {
      usertype += argv[i];
      usertype.resize(usertype.length()+1, ' ');
    }
    else
      break;
  }

  if (!usertype.empty())
  {
    usertype.resize(usertype.length()-1);

    dump_funptr fptr = dump_funcs[ usertype ];
    if (!fptr) throw std::runtime_error("type not supported");

    fptr(argc, argv, i, std::cout);
  }

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
