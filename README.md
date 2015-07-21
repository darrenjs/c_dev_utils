C Dev Utils
===========

c_dev_utils is a collection of simple utility programs for displaying
information regarding the kernel and C\C++ programming environment on GNU\Linux
bases systems.

# cval

Given a C\C++ type, cval will display its limits (as defined by C++), and given
values, will show their hex and binary representation. This is typically of
interest to developers who need understand the machine representation of the
primitive types.

    $ cval short 256 257 258
    short, sizeof 2, non-sign-bits 15, digits10 4, min -32768, max 32767
    256 | 01 00 | 00000001 00000000
    257 | 01 01 | 00000001 00000001
    258 | 01 02 | 00000001 00000010


    $  cval64  unsigned int
    unsigned int, sizeof 4, non-sign-bits 32, digits10 9, min 0, max 4294967295

#cerror

cerror lists many of the standard error codes defined on a Linux system, eg,
values like EWOULDBLOCK, and their assoicated system error strings.


    $ cerror | grep TIME
    ETIME, 62, Timer expired
    ETIMEDOUT, 110, Connection timed out


#cdatetime


Converts user supplieds time_t values (epoch values) into their corresponding
human readale value. Also displays the epoch times for the current moment, and
for start of day.


    $ cdatetime 1437316800
    now                 :           1437370582 : Mon Jul 20 06:36:22 2015
    start of day        :           1437346800 : Mon Jul 20 00:00:00 2015
    1437316800          :           1437316800 : Sun Jul 19 15:40:00 2015
