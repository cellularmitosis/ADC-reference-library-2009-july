#!/bin/bash

set -e -o pipefail
set -x

#for f in $(cat /tmp/out | awk '{print $1}') ; do
#    echo $f
#    iconv -f ISO-8859-1 -t UTF-8 $f > /tmp/fixed
#    mv /tmp/fixed $f
#done

while IFS= read -r line; do
    echo "$line"
    iconv -f ISO-8859-1 -t UTF-8 "$line" > /tmp/fixed
    mv /tmp/fixed "$line"
done < /tmp/out2
