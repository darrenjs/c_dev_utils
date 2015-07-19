#!/bin/sh

T=`mktemp`

if [ -e  /usr/include/asm-generic/errno.h ] ;
then
    cat /usr/include/asm-generic/errno.h | egrep -o -w "E[A-Z]+"  >> $T
fi
man -S 3 -P cat errno | egrep -o -w "E[A-Z]+"  >> $T

for F in send recv open poll select pthread_create clone mmap malloc \
    gettimeofday time

do
    man -S 2 -P cat $F | egrep -o -w "E[A-Z]+"   >> $T
    man -S 3 -P cat $F | egrep -o -w "E[A-Z]+"   >> $T
done

cat $T | sort | uniq > names.txt
