C Dev Utils
===========

c_dev_utils is a collection of simple utility programs for displaying
information regarding the C\C++ programming environment on GNU\Linux systems.

cval
----

Display numerical limits associated with the C\C++ primitive types (as defined
by C++), and the hex and binary representation for specified values.

    $ cval short 256 257 258
    short, sizeof 2, non-sign-bits 15, digits10 4, min -32768, max 32767
    256 | 01 00 | 00000001 00000000
    257 | 01 01 | 00000001 00000001
    258 | 01 02 | 00000001 00000010

    $ cval64 unsigned int
    unsigned int, sizeof 4, non-sign-bits 32, digits10 9, min 0, max 4294967295

    $ cval float inf
    float, sizeof 4, non-sign-bits 24, digits10 6, min 1.175494350822287508e-38, max 3.4028234663852885981e+38
                              inf | 7F 80 00 00 | 01111111 10000000 00000000 00000000


chex
----

The opposite of cval; convert hex strings into the values of C/C++ types.

    $ chex -t "unsigned int,char" 2A
    char (size 1)
    2A | 42   '*'
    unsigned int (size 4)
    00 00 00 2A | 42


cerror
------

Lists many of the standard error codes defined on a Linux system, eg, values
like EWOULDBLOCK, and their assoicated system error strings.


    $ cerror | grep TIME
    ETIME, 62, Timer expired
    ETIMEDOUT, 110, Connection timed out


cdatetime
---------

Convert time_t values (describing epoch time values) into their corresponding
calender value. Also displays the epoch times for the current moment and start
of day.


    $ cdatetime 1437316800
    now                 :           1437370582 : Mon Jul 20 06:36:22 2015
    start of day        :           1437346800 : Mon Jul 20 00:00:00 2015
    1437316800          :           1437316801 : Sun Jul 19 15:40:01 2015
