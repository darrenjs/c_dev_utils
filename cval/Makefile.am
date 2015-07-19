
# In the below, we will always build a 'cval' default binary.  Additionally, if
# we can build a 32 bit verson and 64 bit version, build them with appropriate
# suffixes.

bin_PROGRAMS = cval
cval_LDFLAGS= $(AM_STATIC)
cval_SOURCES = cval.cc


if BUILD32
bin_PROGRAMS += cval32
endif

if BUILD64
bin_PROGRAMS += cval64
endif

cval32_CPPFLAGS=-m32
cval32_LDFLAGS=-m32 $(AM_STATIC)
cval32_SOURCES = cval.cc

cval64_CPPFLAGS=-m64
cval64_LDFLAGS=-m64 $(AM_STATIC)
cval64_SOURCES = cval.cc