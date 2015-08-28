# -*- Autoconf -*-

#===== Initialisation =====

AC_PREREQ([2.59])
AC_INIT([c_dev_utils], [1.0], [cval@darrenjs.net])

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([cval/cval.cc])
AC_CONFIG_SRCDIR([cerror/cerror.c])
AC_CONFIG_SRCDIR([cdatetime/cdatetime.c])

AC_CONFIG_HEADERS([config.h])

#===== Checks for programs =====

AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_INSTALL

# Checks for libraries.

#===== Checks for header files =====
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for library functions.
AC_CHECK_FUNCS([memset exit])


#===== Custom checks =====

# Check we can statically link
my_save_cflags="$CFLAGS"
CFLAGS=-static
AC_MSG_CHECKING([whether can build using -static])
AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
    [AC_MSG_RESULT([yes])]
    [XX_STATIC=-static],
    [AC_MSG_RESULT([no])]
)
AM_CONDITIONAL([BUILDSTATIC], [test x$TEST64 = xtrue])
CFLAGS="$my_save_cflags"

# Allow the variable XX_STATIC to be using the Makefile.am
AC_SUBST([XX_STATIC])


# Check we can compile & link 64 bit
my_save_cflags="$CFLAGS"
CFLAGS=-m64
AC_MSG_CHECKING([whether can build using -m64])
AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
    [AC_MSG_RESULT([yes])]
    [TEST64=true],
    [AC_MSG_RESULT([no])]
    [TEST64=false]
)
AM_CONDITIONAL([BUILD64], [test x$TEST64 = xtrue])
CFLAGS="$my_save_cflags"


# Check we can compile & link 32 bit
my_save_cflags="$CFLAGS"
CFLAGS=-m32
AC_MSG_CHECKING([whether can build using -m32])
AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
    [AC_MSG_RESULT([yes])]
    [TEST32=true],
    [AC_MSG_RESULT([no (g++-multilib installed?)])]
    [TEST32=false]
)
AM_CONDITIONAL([BUILD32], [test x$TEST32 = xtrue])
CFLAGS="$my_save_cflags"

#===== Generation =====

AC_CONFIG_FILES([Makefile
                 cval/Makefile
                 cdatetime/Makefile
                 cerror/Makefile
                 chex/Makefile])

# Trigger the generation of our makefiles
AC_OUTPUT
