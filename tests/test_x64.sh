#!/bin/sh
# Run each test and compare
# against known good output

set -e

echo "Testing for target x86_64..."

if [ ! -f ../acc ]
then echo "Need to build ../acc first!"; exit 1
fi

for i in input*
do if [ ! -f "out.$i" ]
   then echo "Can't run test on $i, no answer file!"
   else
     echo -n $i
     ../acc x86_64 $i
     gcc -o out out.s
     ./out > trial.$i
     cmp -s "out.$i" "trial.$i"
     if [ "$?" -eq "1" ]
     then echo ": failed"
       diff -c "out.$i" "trial.$i"
       echo
     else echo ": OK"
     fi
     rm -f out out.s "trial.$i"
   fi
done
