#!/bin/bash

# detect absolute urls which could be converted to relative urls

set -e -o pipefail

find . -name '*.html' -print0 | while IFS= read -r -d $'\0' file; do
    target_paths="css/ documentation/ featuredarticles/ images/ index.html js/ media/ qa/ reference/ referencelibrary/ releasenotes/ samplecode/ style.css technicalnotes/ technicalqas/ technotes/"
    for path in $target_paths ; do
        if egrep -q -e "[hH][rR][eE][fF]=\"/${path}" "$file" ; then
            echo $file $path
        fi
    done
done
