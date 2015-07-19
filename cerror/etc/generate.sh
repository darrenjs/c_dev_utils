#!/bin/sh

file=names.txt
out=../cerror_generated.c

echo reading $file
for E in `cat $file`
do
    echo '#ifdef' $E
    echo '  OUT(' $E ');'
    echo '#endif'
done > $out
echo wrote $out
