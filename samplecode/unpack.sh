#!/bin/bash

set -e -o pipefail

for p in `find . -name '*.zip'` ; do
d=`dirname $p`
f=`basename $p`
echo "cd $d && unzip $f"
cd $d && unzip $f
cd -
done
